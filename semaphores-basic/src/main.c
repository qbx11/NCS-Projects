
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "ncs_version.h"
#if NCS_VERSION_NUMBER < 0x20600
#include <zephyr/random/rand32.h>
#else 
#include <zephyr/random/random.h>
#endif
#include <string.h>

#define PRODUCER_STACKSIZE 1024
#define CONSUMER_STACKSIZE 1024

#define PRODUCER_PRIORITY 5
#define CONSUMER_PRIORITY 5

//   struct k_sem semafor_1 = {
//       .wait_q = /* pusta kolejka czekających wątków */,
//       .count  = 10,   // initial_count
//       .limit  = 10,   // count_limit
//   };


/* Semafor = licznik + kolejka czekajacych watkow (struct k_sem { count; limit; }).
 * K_SEM_DEFINE(name, initial_count, count_limit) tworzy go STATYCZNIE (juz gotowy
 * przed main(), bez wywolywania k_sem_init()).
 *   initial_count - wartosc startowa licznika (ile "sztuk" dostepnych od razu)
 *   count_limit   - sufit, do ktorego licznik moze urosnac przez k_sem_give()
 * initial_count musi byc <= count_limit, inaczej blad juz na etapie kompilacji. */
K_SEM_DEFINE(instance_monitor_sem, 10,10);

volatile uint32_t available_instance_count = 10;

void get_access(void)
{
	// available_instance_count--;
	// printk("Resource taken and available_instance_count = %d\n", available_instance_count);
	/* k_sem_take(sem, timeout) = "zajmij jedna sztuke":
	 *   count > 0  -> count--, watek leci dalej natychmiast
	 *   count == 0 -> watek sie BLOKUJE (stan Waiting, zero CPU) i czeka w kolejce
	 * Drugi argument (timeout) decyduje, jak dlugo watek ma czekac w tym drugim przypadku:
	 *   K_FOREVER   - czekaj bez limitu, wroc dopiero po realnym wzieciu semafora
	 *   K_NO_WAIT   - nie czekaj wcale, sprawdz raz i wroc od razu (jedyna legalna opcja w ISR)
	 *   K_MSEC(500) - czekaj max tyle ms, po timeoucie zwraca -EAGAIN zamiast wisiec wiecznie
	 * K_FOREVER ma sens, gdy watek i tak nie ma nic innego do zrobienia bez tego zasobu -
	 * ale jesli druga strona (ta co ma zrobic k_sem_give) sama sie zawiesi, ten watek
	 * zablokuje sie na zawsze. */
	k_sem_take(&instance_monitor_sem, K_FOREVER);
	/* k_sem_count_get() tylko ODCZYTUJE count, nic nie zmienia - do samego logowania/debugu */
	printk("Resource taken and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));
	
}

void release_access(void)
{
	/* k_sem_give(sem) = "oddaj jedna sztuke": count++ (do sufitu = limit),
	 * a jesli ktos aktualnie czeka zablokowany w k_sem_take() na tym semaforze,
	 * od razu budzi jednego z nich (Waiting -> Ready). Dowolny watek moze zrobic
	 * give, nie tylko ten, ktory wczesniej zrobil take - semafor nie ma pojecia
	 * "wlasciciela" (w przeciwienstwie do mutexa). */
	k_sem_give(&instance_monitor_sem);
	printk("Resource given and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));

}

void producer(void)
{
	printk("Producer thread started \r\n");
	while(1){
		release_access();
		k_msleep(sys_rand32_get()%10); //rand: 1-9 ms
	}
}

void consumer(void)
{
	printk("Consumer thread started \r\n");
	while(1){
		get_access();
		k_msleep(sys_rand32_get()%10); //rand: 1-9 ms
	}
}


K_THREAD_DEFINE(producer_id, PRODUCER_STACKSIZE, producer, NULL, NULL, NULL, PRODUCER_PRIORITY, 0, 0);
K_THREAD_DEFINE(consumer_id, CONSUMER_STACKSIZE, consumer, NULL, NULL, NULL, CONSUMER_PRIORITY, 0,0);