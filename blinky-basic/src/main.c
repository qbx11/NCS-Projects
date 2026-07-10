#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Czas uśpienia między przełączeniami stanu diody, w milisekundach (1000 ms = 1 s) */
#define SLEEP_TIME_MS   1000

/* Identyfikator węzła devicetree odpowiadającego aliasowi "led0", zdefiniowanemu w pliku .dts/.overlay danej płytki */
#define LED0_NODE DT_ALIAS(led0)

/*
 * Struktura gpio_dt_spec opisuje pin GPIO podłączony do diody: łączy w sobie
 * wskaźnik na kontroler GPIO, numer pinu oraz flagi (np. polaryzację aktywną),
 * odczytane wprost z devicetree pod aliasem led0. Dzięki temu kod nie zależy
 * od konkretnej płytki - wystarczy, że jej devicetree definiuje alias led0.
 * Błąd kompilacji w tej linii oznacza, że wybrana płytka nie definiuje takiego aliasu.
 */
//tworzymy nowa zmienna led typu gpio_dt_spec
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// ^
// |
// |
// struct gpio_dt_spec {
//     const struct device *port;  // wskaźnik na device kontrolera GPIO
//     gpio_pin_t pin;              // numer pinu
//     gpio_dt_flags_t dt_flags;    // flagi (np. GPIO_ACTIVE_LOW)
// };

int main(void)
{
	int ret;
	bool led_state = true;

	/* Sprawdzenie, czy kontroler GPIO diody jest zainicjalizowany i gotowy do użycia */
	//przekazujemy funkcji gpio_is_ready_dt WSKAZNIK do zmiennej led (&led = adres zmiennej led)
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	/* Konfiguracja pinu jako wyjścia, od razu ustawionego w stan aktywny (dioda zapalona) */
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	while (1) {
		/* Przełączenie stanu pinu na przeciwny (zapalona -> zgaszona lub odwrotnie) */
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		/* Aktualizacja lokalnej zmiennej śledzącej stan diody, wyłącznie na potrzeby logu */
		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		/* Odczekanie zadanego czasu przed kolejnym przełączeniem */
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
