#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "ncs_version.h"
#if NCS_VERSION_NUMBER < 0x20600
#include <zephyr/random/rand32.h>
#else
#include <zephyr/random/random.h>
#endif

#define STACKSIZE 1024
#define PRIORITY  5

struct account {
	const char *name;
	int balance;
};

static struct account acc_a = { .name = "A", .balance = 500 };
static struct account acc_b = { .name = "B", .balance = 500 };

/*
 * ZADANIE: transfer() przenosi "amount" z jednego konta na drugie.
 * Operacja musi zablokowac OBA konta na czas transferu, zeby nikt
 * nie zobaczyl polowicznego stanu (pieniadze juz zniknely z jednego
 * konta, ale jeszcze nie dotarly na drugie).
 *
 * Dwa watki robia transfery w PRZECIWNYCH kierunkach jednoczesnie:
 * worker_ab caly czas przelewa z A do B, worker_ba caly czas przelewa
 * z B do A. Program ma dzialac plynnie w nieskonczonosc.
 *
 * Uwaga: to nie jest tylko "dodaj dwa lock/unlock". Naiwne podejscie
 * (kazdy watek najpierw blokuje "from", potem "to") moze doprowadzic
 * do zakleszczenia (deadlock), bo watki blokuja konta w PRZECIWNEJ
 * kolejnosci wzgledem siebie. Zastanow sie, jak temu zapobiec.
 */

K_MUTEX_DEFINE(mutex_a);
K_MUTEX_DEFINE(mutex_b);

static void transfer(struct account *from, struct k_mutex *from_lock,
		      struct account *to, struct k_mutex *to_lock,
		      int amount)
{
	from->balance -= amount;
	k_busy_wait(200);
	to->balance += amount;

	printk("transfer: %s -> %s (%d) | A=%d B=%d\n",
	       from->name, to->name, amount, acc_a.balance, acc_b.balance);
}

static void worker_ab(void)
{
	while (1) {
		transfer(&acc_a, &mutex_a, &acc_b, &mutex_b, 10);
		k_msleep(sys_rand32_get() % 20);
	}
}

static void worker_ba(void)
{
	while (1) {
		transfer(&acc_b, &mutex_b, &acc_a, &mutex_a, 10);
		k_msleep(sys_rand32_get() % 20);
	}
}

K_THREAD_DEFINE(worker_ab_id, STACKSIZE, worker_ab, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(worker_ba_id, STACKSIZE, worker_ba, NULL, NULL, NULL, PRIORITY, 0, 0);
