#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "ncs_version.h"
#if NCS_VERSION_NUMBER < 0x20600
#include <zephyr/random/rand32.h>
#else
#include <zephyr/random/random.h>
#endif
#include <string.h>
#include <stdio.h>

#define STACKSIZE 1024

#define WORKER_COUNT     3
#define WORKER_PRIORITY  5
#define REPORTER_PRIORITY 5

/* Wspolny stan "konta" - kilka POWIAZANYCH ze soba pol. Realistyczna roznica
 * wzgledem prostego licznika: tu trzeba chronic CALY zestaw pol naraz, bo
 * czesciowa (rozdzielona) aktualizacja/odczyt daje niespojny obraz stanu -
 * nie tylko zle liczby, ale np. tez "poszarpany" tekst w last_op. */
struct account {
	int32_t balance;
	uint32_t transaction_count;
	char last_op[32];
};

static struct account acc = {
	.balance = 0,
	.transaction_count = 0,
	.last_op = "none",
};

/* TODO 1 - zdefiniuj mutex chroniacy dostep do "acc" (makro do statycznego
 * tworzenia mutexa, patrz K_SEM_DEFINE ktorego juz uzywales - dla mutexa
 * jest analogiczny odpowiednik, tylko bez initial_count/limit) */
K_MUTEX_DEFINE(account_mutex);

static void apply_transaction(int worker_id, int32_t amount)
{
	/* TODO 2 - zablokuj mutex PRZED dotknieciem "acc" (bez limitu czasu -
	 * kazdy worker i tak nie ma nic innego do roboty bez dostepu do konta) */
	k_mutex_lock(&account_mutex, K_FOREVER);

	acc.balance += amount;
	acc.transaction_count++;
	snprintf(acc.last_op, sizeof(acc.last_op), "worker%d: %s%d",
		 worker_id, amount >= 0 ? "+" : "", amount);

	/* Symulacja realnej pracy (np. zapis do "bazy danych"/EEPROM) -
	 * celowo poszerza okno czasowe, w ktorym inny watek moglby wejsc
	 * w polowe aktualizacji, gdyby brakowalo ochrony. */
	k_busy_wait(300);


	/* TODO 3 - odblokuj mutex PO zakonczeniu modyfikacji "acc" */
	k_mutex_unlock(&account_mutex);

}

static void worker_thread(void *id_arg, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	int id = (int)(intptr_t)id_arg;

	while(1) {
		int32_t amount = (int32_t)(sys_rand32_get() % 21) - 10; /* -10..+10 */

		apply_transaction(id, amount);
		k_msleep(sys_rand32_get() % 30);
	}for (int i = 0; i < 15; i++)

	printk("worker%d finished\n", id);
}

static void reporter_thread(void)
{
	while (1) {
		struct account snapshot;

		/* TODO 4 - zrob SPOJNY zrzut "acc" do "snapshot":
		 *   1. zablokuj mutex
		 *   2. skopiuj WSZYSTKIE pola "acc" do "snapshot" (np. snapshot = acc;)
		 *   3. odblokuj mutex
		 * Wypisywanie (printk ponizej) celowo zostaw JUZ POZA sekcja
		 * chronioną mutexem - printk jest wolny, a mutex ma byc trzymany
		 * jak najkrocej. */
		k_mutex_lock(&account_mutex,K_FOREVER);

		snapshot = acc;

		k_mutex_unlock(&account_mutex);


		printk("--- account snapshot: balance=%d, transactions=%u, last_op=\"%s\" ---\n",
		       snapshot.balance, snapshot.transaction_count, snapshot.last_op);

		k_msleep(300);
	}
}

K_THREAD_DEFINE(worker0_id, STACKSIZE, worker_thread, (void *)0, NULL, NULL, WORKER_PRIORITY, 0, 0);
K_THREAD_DEFINE(worker1_id, STACKSIZE, worker_thread, (void *)1, NULL, NULL, WORKER_PRIORITY, 0, 0);
K_THREAD_DEFINE(worker2_id, STACKSIZE, worker_thread, (void *)2, NULL, NULL, WORKER_PRIORITY, 0, 0);
K_THREAD_DEFINE(reporter_id, STACKSIZE, reporter_thread, NULL, NULL, NULL, REPORTER_PRIORITY, 0, 0);
