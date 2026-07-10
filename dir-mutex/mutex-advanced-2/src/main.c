#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "ncs_version.h"
#if NCS_VERSION_NUMBER < 0x20600
#include <zephyr/random/rand32.h>
#else
#include <zephyr/random/random.h>
#endif

#define STACKSIZE 1024

#define SENSOR_PRIORITY 5
#define LOGGER_PRIORITY 5

#define RING_SIZE 8

#define SENSOR_TEMP     0
#define SENSOR_HUMIDITY 1

/* Scenariusz: dwa "czujniki" (osobne watki) dziela JEDNA fizyczna magistrale
 * (np. I2C) - tylko jeden moze z nia rozmawiac naraz. Kazdy odczyt trafia
 * do wspolnego bufora kolowego o STALYM rozmiarze (typowe dla embedded -
 * zero malloc), z ktorego osobny watek "logger" go wyciaga i wypisuje. */
struct sensor_sample {
	uint8_t sensor_id;
	int16_t value;
};

static struct sensor_sample ring_buf[RING_SIZE];
static int ring_head; /* nastepny wolny slot do zapisu - dotyka go PRODUCENT */
static int ring_tail; /* nastepny slot do odczytu - dotyka go KONSUMENT */

/* TODO 1 - zdefiniuj mutex "bus_mutex" chroniacy dostep do wspolnej,
 * symulowanej magistrali - tylko jeden czujnik na raz moze przez nia
 * "rozmawiac" ze sprzetem. */
K_MUTEX_DEFINE(bus_mutex);


/* TODO 2 - zdefiniuj mutex "ring_mutex" chroniacy indeks ring_head i sam
 * zapis do ring_buf - potrzebny, bo DWA watki-czujniki moga chciec pisac
 * do bufora w tym samym momencie. */
K_MUTEX_DEFINE(ring_mutex);


/* TODO 3 - zdefiniuj semafor liczacy "free_slots", startowy=RING_SIZE,
 * limit=RING_SIZE - ile WOLNYCH miejsc jest teraz w buforze. */
K_SEM_DEFINE(free_slots, RING_SIZE, RING_SIZE);


/* TODO 4 - zdefiniuj semafor liczacy "filled_slots", startowy=0,
 * limit=RING_SIZE - ile ZAPELNIONYCH (gotowych do odczytu) miejsc jest
 * teraz w buforze. */
K_SEM_DEFINE(filled_slots, 0, RING_SIZE);


static int16_t read_sensor_register(uint8_t sensor_id)
{
	/* TODO 5 - zablokuj bus_mutex PRZED "transakcja" z magistrala */
	k_mutex_lock(&bus_mutex, K_FOREVER);

	k_busy_wait(150); /* symulacja czasu trwania transakcji I2C */
	int16_t value = (int16_t)(sys_rand32_get() % 100);

	/* TODO 6 - odblokuj bus_mutex PO transakcji */
	k_mutex_unlock(&bus_mutex);

	return value;
}

static void ring_push(uint8_t sensor_id, int16_t value)
{
	/* TODO 7 - wez semafor free_slots (K_FOREVER) - jesli bufor jest pelny,
	 * watek ma tu zaczekac, az logger zwolni jakies miejsce. */
	k_sem_take(&free_slots, K_FOREVER);

	/* TODO 8 - zablokuj ring_mutex, zapisz probke do ring_buf[ring_head],
	 * zwieksz ring_head modulo RING_SIZE, odblokuj ring_mutex. */


	/* TODO 9 - oddaj semafor filled_slots - zasygnalizuj loggerowi,
	 * ze pojawila sie nowa probka do odczytu. */

}

static void sensor_thread(void *id_arg, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	uint8_t sensor_id = (uint8_t)(uintptr_t)id_arg;

	while (1) {
		int16_t value = read_sensor_register(sensor_id);

		ring_push(sensor_id, value);

		k_msleep(sys_rand32_get() % 100);
	}
}

static void logger_thread(void)
{
	while (1) {
		/* TODO 10 - wez semafor filled_slots (K_FOREVER) - jesli bufor
		 * jest pusty, watek ma tu zaczekac, az jakis czujnik cos doda. */


		struct sensor_sample sample = ring_buf[ring_tail];

		ring_tail = (ring_tail + 1) % RING_SIZE;

		/* TODO 11 - oddaj semafor free_slots - zwolniles jedno miejsce
		 * w buforze, powiedz o tym czujnikom czekajacym w ring_push(). */


		printk("logger: sensor%u = %d\n", sample.sensor_id, sample.value);
	}
}

K_THREAD_DEFINE(sensor_temp_id, STACKSIZE, sensor_thread, (void *)SENSOR_TEMP, NULL, NULL,
		SENSOR_PRIORITY, 0, 0);
K_THREAD_DEFINE(sensor_humidity_id, STACKSIZE, sensor_thread, (void *)SENSOR_HUMIDITY, NULL, NULL,
		SENSOR_PRIORITY, 0, 0);
K_THREAD_DEFINE(logger_id, STACKSIZE, logger_thread, NULL, NULL, NULL, LOGGER_PRIORITY, 0, 0);
