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

/*
 * ZADANIE: kazdy z trzech watkow co jakis czas wypisuje na konsole
 * wielolinijkowy "raport" (patrz print_report()). Bez ochrony linie
 * raportow z roznych watkow moga sie ze soba przeplatac.
 *
 * Dodaj synchronizacje, ktora zagwarantuje, ze kazdy raport wypisuje sie
 * w calosci, bez przeplatania z raportem innego watku.
 */

 K_MUTEX_DEFINE(print_mutex);

static void print_report(int worker_id, int value)
{
	k_mutex_lock(&print_mutex, K_FOREVER);

	printk("=== report from worker%d ===\n", worker_id);
	k_msleep(5);
	printk("  value = %d\n", value);
	k_msleep(5);
	printk("=== end of worker%d report ===\n", worker_id);

	k_mutex_unlock(&print_mutex);

}

static void worker_thread(void *id_arg, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	int id = (int)(intptr_t)id_arg;

	while (1) {
		int value = (int)(sys_rand32_get() % 100);

		print_report(id, value);

		k_msleep(sys_rand32_get() % 50);
	}
}

K_THREAD_DEFINE(worker0_id, STACKSIZE, worker_thread, (void *)0, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(worker1_id, STACKSIZE, worker_thread, (void *)1, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(worker2_id, STACKSIZE, worker_thread, (void *)2, NULL, NULL, PRIORITY, 0, 0);
