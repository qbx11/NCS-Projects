#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Czas uśpienia między przełączeniami stanu diody, w milisekundach (1000 ms = 1 s) */
#define SLEEP_TIME_MS   1000

/* Identyfikator węzła devicetree odpowiadającego aliasowi "led0", zdefiniowanemu w pliku .dts/.overlay danej płytki */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)


/*
 * Struktura gpio_dt_spec opisuje pin GPIO podłączony do diody: łączy w sobie
 * wskaźnik na kontroler GPIO, numer pinu oraz flagi (np. polaryzację aktywną),
 * odczytane wprost z devicetree pod aliasem led0. Dzięki temu kod nie zależy
 * od konkretnej płytki - wystarczy, że jej devicetree definiuje alias led0.
 * Błąd kompilacji w tej linii oznacza, że wybrana płytka nie definiuje takiego aliasu.
 */
 static const struct gpio_dt_spec leds[] = {
		GPIO_DT_SPEC_GET(LED0_NODE, gpios),
		GPIO_DT_SPEC_GET(LED1_NODE, gpios),
	};

int main(void)
{
	int ret;

	/* Sprawdzenie, czy kontroler GPIO diody jest zainicjalizowany i gotowy do użycia */
	//przekazujemy funkcji gpio_is_ready_dt WSKAZNIK do zmiennej led (&led = adres zmiennej led)
	for (size_t i=0;i < ARRAY_SIZE(leds); i++){
		if(!gpio_is_ready_dt(&leds[i])){
			return 0;
		}
	}
	/* Konfiguracja pinu jako wyjścia, od razu ustawionego w stan aktywny (GPIO_OUTPUT_ACTIVE -> dioda zapalona) */
	for (size_t i=0;i < ARRAY_SIZE(leds); i++){
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			return 0;
		}
	}

	
	while (1) {
		for(size_t i=0;i<ARRAY_SIZE(leds);i++){
			ret = gpio_pin_toggle_dt(&leds[i]);
			if (ret < 0) {
				return 0;
			}
		}	
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
