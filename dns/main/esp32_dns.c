#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include "tcpip_adapter.h"

#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"

#include "nvs_flash.h"
#include "dns_server.h"
#include "web_server.h"

#define TAG  "dns_server"

#define CONFIG_WIFI_SSID      "ESP32-WIFI"
#define CONFIG_WIFI_PASSWORD  "12345678"

//wifi状态机回调函数
static esp_err_t event_handler(void *ctx,system_event_t *event)
{
	switch(event->event_id){
		case SYSTEM_EVENT_AP_STACONNECTED:
		    ESP_LOGI(TAG,"station:"MACSTR" join,AID=%d\n",MAC2STR(event->event_info.sta_connected.mac),
			event->event_info.sta_connected.aid);
			break;

		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG,"Wifi disconnected,try to connect ...");
			esp_wifi_connect();
			break;

		default:
			break;
	}

	return ESP_OK;
}

void initilalise_wifi(void)
{
	nvs_flash_init();
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler,NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	wifi_config_t app_config = {
		.ap = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASSWORD,
			.ssid_len = 0,
			.max_connection = 4,
			.authmode = WIFI_AUTH_WPA_PSK
		}
	};

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&app_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	esp_wifi_connect();
}


void app_main()
{
	initilalise_wifi();
	my_udp_init();
	xTaskCreate(&web_server2,"web_server2",2048*2,NULL,5,NULL);
}



