#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
/* STEP 3 - Include the header file of the Bluetooth LE stack */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>


#include <dk_buttons_and_leds.h>

/* Rejestruje moduł logowania - pozwala na używanie LOG_INF/LOG_ERR z nazwą "Lesson2_Exercise1" */
LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);

/* Nazwa urządzenia BLE - wartość ustawiona w prj.conf (CONFIG_BT_DEVICE_NAME) */
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
/* Długość nazwy bez końcowego znaku '\0' (sizeof liczy go, ale nie jest on wysyłany w pakiecie BLE) */
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

/* STEP 4.1.1 - Declare the advertising packet */
/* Pakiet reklamowy (advertising data) - dane rozgłaszane cyklicznie przez urządzenie */
static const struct bt_data ad[] = {
	/* Flaga "Flags": urządzenie obsługuje tylko BLE, bez klasycznego Bluetootha (BR/EDR) */
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	/* Pełna nazwa urządzenia, widoczna dla skanujących urządzeń (np. w aplikacji na telefonie) */
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};


/* STEP 4.2.2 - Declare the URL data to include in the scan response */
/* Adres URL w formacie Eddystone-URL: pierwszy bajt 0x17 to prefiks "https://", reszta to "academy.nordicsemi.com" */
static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
                                 'n','o','r','d','i','c','s','e','m','i','.',
                                 'c','o','m'};

/* STEP 4.2.1 - Declare the scan response packet */
/* Pakiet odpowiedzi skanowania (scan response) - dodatkowe dane wysyłane na żądanie skanującego urządzenia */
static const struct bt_data sd[] = {
	/* Dane typu URI (adres URL) z tablicy url_data zdefiniowanej powyżej */
	BT_DATA(BT_DATA_URI, url_data,sizeof(url_data)),

};

int main(void)
{
	int blink_status = 0;
	int err;


	/* Inicjalizacja diod LED na płytce (potrzebne do migania diodą "run status") */
	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}


	/* STEP 5 - Enable the Bluetooth LE stack */
	/* Włącza stos Bluetooth LE - musi być wywołane przed jakąkolwiek inną operacją BLE */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("BLE init failed (err %d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");

	/* STEP 6 - Start advertising */
	/* Rozpoczyna rozgłaszanie (advertising) pakietami ad/sd zdefiniowanymi wyżej.
	 * BT_LE_ADV_NCONN oznacza reklamowanie bez możliwości nawiązania połączenia (tzw. non-connectable) */
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}

	LOG_INF("Advertising successfully started\n");

	/* Główna pętla programu - miga diodą RUN_STATUS_LED, sygnalizując że urządzenie działa */
	for (;;) {
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
