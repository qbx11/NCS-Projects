#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACKSIZE 1024
#define PRIORITY  5

/* K_MUTEX_DEFINE(name) - tworzy mutex STATYCZNIE, juz odblokowany, gotowy przed main().
* W przeciwienstwie do K_SEM_DEFINE nie potrzebuje initial_count/limit - mutex
* zawsze startuje w jednym, oczywistym stanie: nikt go nie trzyma. */
K_MUTEX_DEFINE(counter_mutex);

static int shared_counter = 0;

static void increment_safely(void)
{
    /* k_mutex_lock(mutex, timeout) - jesli nikt nie trzyma, ten watek od razu
    * zostaje wlascicielem i leci dalej. Jesli ktos trzyma, blokuje sie (Waiting)
    * i czeka w kolejce, az wlasciciel zrobi unlock. */
    k_mutex_lock(&counter_mutex, K_FOREVER);

    /* Sekcja krytyczna - tylko JEDEN watek naraz moze tu wejsc. */
    int tmp = shared_counter;
    k_busy_wait(100); /* sztucznie poszerza okno czasowe, w ktorym race condition
                        * moglby sie ujawnic - usun mutex ponizej, zeby to zobaczyc */
    shared_counter = tmp + 1;

    /* k_mutex_unlock - TYLKO wlasciciel moze to zrobic. Budzi jednego
        * z czekajacych watkow, jesli ktos czekal. */
    k_mutex_unlock(&counter_mutex);
}


void thread_a(void)
{     
    for (int i = 0; i < 20; i++) {
        increment_safely();
        printk("thread_a: shared_counter = %d\n", shared_counter);
    }       
}

void thread_b(void)
{     
    for (int i = 0; i < 20; i++) {
        increment_safely();
        printk("thread_b: shared_counter = %d\n", shared_counter);
    }       
}

K_THREAD_DEFINE(thread_a_id, STACKSIZE, thread_a, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(thread_b_id, STACKSIZE, thread_b, NULL, NULL, NULL, PRIORITY, 0, 0);