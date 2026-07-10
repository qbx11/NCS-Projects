#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/*
 * Czas uśpienia pętli głównej w milisekundach. Wątek main() i tak nie robi
 * nic poza spaniem - całą logiką przełączania diody zajmuje się przerwanie
 * obsługiwane w button_pressed(), więc ta wartość wpływa jedynie na to, jak
 * często wątek się budzi (a nie na responsywność przycisku).
 */
#define SLEEP_TIME_MS 100

/*
 * SW0_NODE to identyfikator węzła devicetree wskazywanego przez alias "sw0"
 * (zdefiniowany zwykle w pliku .dts/.overlay danej płytki). DT_ALIAS pozwala
 * odwołać się do pinu przycisku niezależnie od konkretnego mikrokontrolera.
 */
#define SW0_NODE DT_ALIAS(sw0)
/*
 * gpio_dt_spec to struktura opisująca pin GPIO na podstawie danych z
 * devicetree: wskaźnik na urządzenie (port GPIO), numer pinu oraz flagi
 * (np. aktywny stan, podciąganie). GPIO_DT_SPEC_GET wyciąga te dane
 * automatycznie z właściwości "gpios" węzła sw0.
 */
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

/*
 * LED0_NODE mimo nazwy wskazuje tutaj na alias "led1" - czyli druga dioda
 * na płytce jest tą, którą będziemy przełączać po wciśnięciu przycisku.
 */
#define LED0_NODE DT_ALIAS(led1)
/* Analogicznie jak wyżej - opis pinu diody LED pobrany z devicetree. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/*
 * Funkcja obsługi przerwania (callback) wywoływana automatycznie przez
 * sterownik GPIO, gdy na pinie przycisku wystąpi skonfigurowane zdarzenie
 * (tutaj: zbocze narastające do stanu aktywnego). Zostaje ona wykonana
 * w kontekście przerwania, dlatego powinna być krótka i nieblokująca -
 * tutaj jedynie przełącza stan diody LED.
 *
 * Parametry:
 *   dev  - urządzenie GPIO, które zgłosiło przerwanie,
 *   cb   - wskaźnik na strukturę callbacku, dzięki której sterownik wie,
 *          którą funkcję wywołać (to ta sama struktura co button_cb_data),
 *   pins - bitmaska pinów, które wywołały przerwanie (przydatne, gdy jeden
 *          callback obsługuje wiele pinów na tym samym porcie).
 */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&led);
}

/*
 * Struktura button_cb_data łączy w sobie funkcję obsługi przerwania z
 * informacją o tym, którego pinu dotyczy (ustawianą przez
 * gpio_init_callback poniżej). Musi być statyczna/globalna, ponieważ
 * sterownik GPIO trzyma do niej wskaźnik przez cały czas działania
 * programu - nie może to być zmienna lokalna, która zniknęłaby po
 * wyjściu z main().
 */
static struct gpio_callback button_cb_data;

int main(void)
{
	int ret;

	/*
	 * Przed użyciem jakiegokolwiek urządzenia z devicetree należy sprawdzić,
	 * czy sterownik zdążył się poprawnie zainicjalizować (device_is_ready
	 * zwraca false np. gdy sterownik nie skompilował się do obrazu albo
	 * inicjalizacja się nie powiodła). Dotyczy to portu GPIO, do którego
	 * podłączona jest dioda LED.
	 */
	if (!device_is_ready(led.port)) {
		return -1;
	}

	/* To samo sprawdzenie, ale dla portu GPIO obsługującego przycisk. */
	if (!device_is_ready(button.port)) {
		return -1;
	}

	/*
	 * Konfigurujemy pin diody jako wyjście (GPIO_OUTPUT) i od razu
	 * ustawiamy go w stan aktywny (ACTIVE) - flaga GPIO_OUTPUT_ACTIVE
	 * powoduje, że dioda zaświeci się zaraz po starcie programu.
	 */
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}

	/* Konfigurujemy pin przycisku jako wejście, aby móc odczytywać jego stan. */
	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return -1;
	}

	/*
	 * Włączamy generowanie przerwania na pinie przycisku. GPIO_INT_EDGE_TO_ACTIVE
	 * oznacza, że przerwanie zostanie zgłoszone w momencie przejścia pinu
	 * ze stanu nieaktywnego w aktywny (czyli np. w chwili wciśnięcia
	 * przycisku, zależnie od polaryzacji zdefiniowanej w devicetree).
	 * Uwaga: wynik tej funkcji nie jest tutaj sprawdzany - w kodzie
	 * produkcyjnym warto dodać taką samą kontrolę błędu jak wyżej.
	 */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);


	/*
	 * Inicjalizujemy strukturę callbacku: wiążemy ją z funkcją
	 * button_pressed oraz z konkretnym pinem przycisku (BIT(button.pin)
	 * tworzy maskę bitową z jednym ustawionym bitem odpowiadającym temu
	 * pinowi). Ta operacja jedynie przygotowuje strukturę w pamięci -
	 * nie rejestruje jeszcze callbacku w sterowniku GPIO.
	 */
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

	/*
	 * Rejestrujemy przygotowaną strukturę callbacku w sterowniku portu
	 * GPIO przycisku. Od tego momentu wystąpienie skonfigurowanego wyżej
	 * przerwania spowoduje wywołanie funkcji button_pressed().
	 */
	gpio_add_callback(button.port, &button_cb_data);

	/*
	 * Pętla główna nie robi nic poza usypianiem wątku - cała logika
	 * reagowania na przycisk odbywa się asynchronicznie w callbacku
	 * przerwania button_pressed(). Wątek main mógłby tu w przyszłości
	 * wykonywać inne zadania w tle (np. obsługę czujników, komunikację),
	 * korzystając z faktu, że obsługa przycisku nie blokuje procesora.
	 */
	while (1) {


		k_msleep(SLEEP_TIME_MS);
	}
}