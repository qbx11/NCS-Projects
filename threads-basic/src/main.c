#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* STEP 2 - Rozmiar stosu (w bajtach) rezerwowany dla KAZDEGO z watkow osobno -
 * statyczna tablica tworzona na etapie kompilacji (zob. K_THREAD_STACK_DEFINE
 * wewnatrz makra K_THREAD_DEFINE ponizej). Oba watki maja ROWNY priorytet 7 -
 * to celowe, zeby pokazac co sie dzieje, gdy priorytety sa takie same. */
#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7

void thread0_func(void)
{
	while (1) {
		/* STEP 3 - printk (nie printf!) - dziala bez malloc, bezpieczne tez w ISR */
		printk("Hello, I am Thread 0 \r\n");
		/* STEP 6 - k_msleep(5) usypia TEN watek na 5ms - w tym czasie kernel
		 * moze uruchomic inny gotowy watek (tu: thread1_func). Bez tego, przy
		 * CONFIG_TIMESLICING=n (zob. prj.conf) i rownym priorytecie obu watkow,
		 * kernel NIE przelaczalby sie sam miedzy nimi - jeden moglby zawlaszczyc
		 * CPU na zawsze, nigdy nie oddajac go drugiemu. */
		k_msleep(5);
		/* STEP 10 - docelowo tutaj wstawia sie WLASCIWY czas usypiania (np. 1000ms,
		 * "co ile faktycznie ma sie powtarzac wypisywanie"), a powyzsze k_msleep(5)
		 * z kroku 6 sluzylo tylko do zademonstrowania oddawania CPU */
		/* Remember to comment out the line from STEP 6 */
	}
}

void thread1_func(void)
{
	while (1) {
		/* STEP 3 - jak wyzej w thread0_func */
		printk("Hello, I am Thread 1 \r\n");
		/* STEP 8 - analogicznie do STEP 6 w thread0_func - patrz komentarz tam */
		k_msleep(5);
		/* STEP 10 - Put the thread to sleep */
		/* Remember to comment out the line from STEP 8 */
	}
}

/* STEP 4 - Statyczne utworzenie obu watkow (na etapie kompilacji, nie w main()).
 * Pelny opis kazdego z 9 parametrow
 * K_THREAD_DEFINE(name, stack_size, entry, p1, p2, p3, prio, options, delay)
 * znajduje sie w tabelce ponizej. */
K_THREAD_DEFINE(thread_0, STACKSIZE, thread0_func, NULL, NULL, NULL, THREAD0_PRIORITY,0,0);
K_THREAD_DEFINE(thread_1, STACKSIZE, thread1_func, NULL, NULL, NULL, THREAD1_PRIORITY,0,0);


/*
  ┌─────┬────────────┬───────────────────────────────────────────────────────────────────────────────────────────────────────────┬────────────────────────────────────────────────────────┐
  │  #  │  Parametr  │                                                Co to jest                                                 │             Co wstawiasz w Twoim przypadku             │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 1   │ name       │ Nazwa, którą sam wymyślasz — stanie się identyfikatorem tego wątku (jeśli kiedyś będziesz chciał się do   │ thread0_id                                             │
  │     │            │ niego odwołać, np. k_thread_priority_set(thread0_id, ...))                                                │                                                        │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 2   │ stack_size │ Rozmiar stosu w bajtach (to, co omawialiśmy wcześniej)                                                    │ STACKSIZE (macie już zdefiniowane w STEP 2)            │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 3   │ entry      │ Funkcja, która będzie ciałem wątku — to, co ma się wykonywać                                              │ thread0 (nazwa Waszej funkcji)                         │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 4-6 │ p1, p2, p3 │ Do trzech argumentów przekazywanych do funkcji entry przy starcie                                         │ thread0 nie przyjmuje żadnych argumentów (void         │
  │     │            │                                                                                                           │ thread0(void)), więc: NULL, NULL, NULL                 │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 7   │ prio       │ Priorytet wątku (mniejsza liczba = wyższy priorytet, jak omawialiśmy przy cooperative/preemptible)        │ THREAD0_PRIORITY (macie już zdefiniowane)              │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 8   │ options    │ Flagi specjalne (np. K_ESSENTIAL), łączone przez |                                                        │ 0 — brak specjalnych opcji, standardowy przypadek      │
  ├─────┼────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────┼────────────────────────────────────────────────────────┤
  │ 9   │ delay      │ Ile milisekund poczekać po starcie systemu, zanim wątek ruszy                                             │ 0 — start natychmiastowy, bez opóźnienia               │
  └─────┴────────────┴───────────────────────────────────────────────────────────────────────────────────────────────────────────┴────────────────────────────────────────────────────────┘
*/