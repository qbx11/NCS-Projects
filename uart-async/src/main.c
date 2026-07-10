#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>



#define SLEEP_TIME_MS 1000

/* DT_ALIAS() - alias zdefiniowany w devicetree PLYTKI (aliases{} node).
 * To "rola" (np. led1), a nie surowa nazwa sprzetowa - dzieki temu ten sam
 * kod dziala na innych plytkach, o ile one tez definiuja taki alias. */
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define LED4_NODE DT_ALIAS(led4)

#define RECEIVE_BUFF_SIZE 10
/* Czas bezczynnosci (w mikrosekundach) po ostatnim odebranym bajcie, po
 * ktorym driver i tak zglosi UART_RX_RDY - nawet jesli bufor sie nie zapelnil. */
#define RECEIVE_TIMEOUT 100


/* GPIO_DT_SPEC_GET zwraca CALA STRUKTURE (nie wskaznik!): { port, pin, dt_flags }.
 * GPIO dziala na pojedynczych pinach w ramach portu, wiec sam wskaznik na
 * urzadzenie by nie wystarczyl - trzeba wiedziec jeszcze KTORY to pin. */
static const struct gpio_dt_spec leds[] ={
	GPIO_DT_SPEC_GET(LED1_NODE, gpios),
	GPIO_DT_SPEC_GET(LED2_NODE, gpios),
	GPIO_DT_SPEC_GET(LED3_NODE, gpios),
	GPIO_DT_SPEC_GET(LED4_NODE, gpios),
};



/* DT_NODELABEL() - odwolanie WPROST do etykiety wezla zapisanej w devicetree
 * SoC (np. "uart0:" w pliku .dtsi chipa), bez posrednictwa aliasu. UART to
 * cale peryferium (nie pojedynczy pin), wiec DEVICE_DT_GET zwraca juz gotowy
 * WSKAZNIK "struct device *" - Zephyr sam stworzyl ten obiekt przy budowaniu,
 * my tylko dostajemy do niego adres (stad brak potrzeby & przy jego uzyciu). */
static const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t tx_buf[] =   {"nRF Connect SDK Fundamentals Course\r\n"
                             "Press 1-4 on your keyboard to toggle LEDS 1-4 on your development kit\r\n"};

/* Bufor, do ktorego UART (w trybie async) sam wpisuje odebrane bajty. */
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

/* Callback UART - sygnatura NARZUCONA przez typ uart_callback_t, driver
 * zawsze wywola ta funkcje z tymi trzema argumentami niezaleznie od tego,
 * co sie stalo. W przeciwienstwie do GPIO (gdzie callback mial jedno
 * znaczenie - "pin sie zmienil"), tutaj JEDEN callback obsluguje WIELE
 * roznych zdarzen (evt->type) - trzeba je rozroznic switchem. */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {

	/* UART_RX_RDY = "sa nowe dane gotowe do odczytu" - odpala sie gdy
	 * bufor sie zapelnil LUB minal RECEIVE_TIMEOUT od ostatniego bajtu.
	 * Moze wystapic wielokrotnie dla tego samego bufora! evt->data.rx.offset
	 * mowi, OD KTOREGO miejsca w buforze zaczynaja sie NOWE dane, a
	 * evt->data.rx.len - ile ich nowych przyszlo. */
	case UART_RX_RDY:
		if ((evt->data.rx.len) == 1) {

			if (evt->data.rx.buf[evt->data.rx.offset] == '1') {
				gpio_pin_toggle_dt(&leds[0]);
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '2') {
				gpio_pin_toggle_dt(&leds[1]);
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '3') {
				gpio_pin_toggle_dt(&leds[2]);
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '4') {
				gpio_pin_toggle_dt(&leds[3]);
			}
		}
		break;
	/* UART_RX_DISABLED = odbior zostal CALKOWICIE wylaczony (np. zabraklo
	 * bufora) i nie bedzie kolejnych zdarzen RX, dopoki sam nie wywolamy
	 * uart_rx_enable() ponownie - stad ta linijka tutaj, a nie w main()/petli. */
	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
		break;

	default:
		break;
	}
}




int main(void)
{
	int ret;

	/* gpio_is_ready_dt(&leds[i]) - wrapper na device_is_ready(spec->port),
	 * dlatego bierze WSKAZNIK na cala strukture gpio_dt_spec (&leds[i]) -
	 * w srodku sam wyciaga .port i dopiero na nim woła device_is_ready(). */
	for(size_t i=0;i < ARRAY_SIZE(leds);i++){
		if(!gpio_is_ready_dt(&leds[i])){
			return 0;
		}
	}
	for(size_t i=0; i < ARRAY_SIZE(leds);i++){
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if(ret<0){
			return 0;
		}
	}

	/* samo "uart" (bez &) bo to JUZ jest wskaznik (struct device *) -
	 * device_is_ready() wlasnie takiego typu oczekuje wprost. */
	if(!device_is_ready(uart)){
		printk("UART device not ready \r\n");
		return 0;
	}

	/* Kolejnosc ma znaczenie:
	 * 1. uart_callback_set - "zarejestruj funkcje, ktora bedzie wolana na
	 *    kazde zdarzenie UART" (odpowiednik gpio_add_callback z GPIO).
	 *    Samo w sobie NIC jeszcze nie wlacza.
	 * 2. uart_tx - wysyla baner raz, przy starcie (nie w petli!).
	 * 3. uart_rx_enable - dopiero TO faktycznie wlacza nasluchiwanie
	 *    (odpowiednik gpio_pin_interrupt_configure_dt z GPIO). Wywolane
	 *    raz tutaj; ponowne wlaczenie po ewentualnym wylaczeniu odbioru
	 *    obsluguje juz sam callback w case UART_RX_DISABLED. */
	ret = uart_callback_set(uart, uart_cb, NULL);
		if (ret) {
			return 1;
		}

	ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
		if (ret) {
			return 1;
		}

	ret = uart_rx_enable(uart ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
		if (ret) {
			return 1;
		}

	/* main() od tego miejsca juz nic nie "robi" - cala logika (odbior
	 * znakow, przelaczanie diod) dzieje sie asynchronicznie w uart_cb(),
	 * wywolywanym przez driver UART w tle. Petla tylko usypia watek. */
	while (1) {
		k_msleep(SLEEP_TIME_MS);
	}
}