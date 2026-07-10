
#include <zephyr/kernel.h> 
#include <zephyr/sys/printk.h>

#define STACKSIZE 1024

/* Rowny priorytet obu watkow = time slicing ma sens tylko miedzy watkami
 * o TYM SAMYM priorytecie. */
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7

void thread0(void)
{
	while (1) {
		printk("Hello, I am thread0\n");
		/* k_busy_wait = aktywne czekanie, watek NIE oddaje CPU dobrowolnie.
		 * Bez time slicingu taki watek moglby zablokowac CPU na zawsze. */
		k_busy_wait(1000000);
	}
}

void thread1(void)
{
	while (1) {
		printk("Hello, I am thread1\n");
		k_busy_wait(1000000);
	}
}

K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);

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