#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_types.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"


static const char *TAG = "example";

#define LED_GPIO_NUM    GPIO_NUM_4

#define CONFIG_AP_SSID       "ESP32_wifi"
#define CONFIG_AP_PASSWORD   "12345678"

//网卡地址
uint8_t ApMac[6];

//wifi状态机回调函数
static esp_err_t event_handler(void *ctx,system_event_t *event)
{
	switch(event->event_id){
		case SYSTEM_EVENT_AP_START:
			//ESP_LOGI(TAG,"Connecting to AP...");
			//esp_wifi_connect();
			printf("\nwifi_softap_start\n");
			break;

		case SYSTEM_EVENT_AP_STACONNECTED:
			//ESP_LOGI(TAG,"Connected.");
			printf("\nwifi_softap_connected\n");
			break;

		case SYSTEM_EVENT_AP_STADISCONNECTED:
			//ESP_LOGI(TAG,"Wifi disconnected,try to connect again...");
			printf("\nwifi_softap_disconnected\n");
			break;

		default:
			break;
	}

	return ESP_OK;
}

void app_main()
{
	//初始化nvs
	ESP_ERROR_CHECK(nvs_flash_init());

	//初始化tcp/ip适配层
	tcpip_adapter_init();

	//设置wifi回调函数，调用event_handler.
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler,NULL));

    //初始化wifi设置函数
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	
	//初始化wifi驱动
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));


	//esp_wifi_get_mac(ESP_IF_WIFI_AP,ApMac);

	wifi_config_t ap_config = {
		.ap = {
			.ssid = CONFIG_AP_SSID,
			.password = CONFIG_AP_PASSWORD,
			.ssid_len = 0,
			.max_connection = 4,
			.authmode = WIFI_AUTH_WPA2_PSK
		},
	};

    //ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_ARM));
	//设置wifi模式为AP 模式
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	//sprintf((char*)ap_config.ap.ssid,"%s_%02x%02x",CONFIG_AP_SSID,ApMac[4],ApMac[5]);
    
	//配置wifi接口参数
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP,&ap_config));
	
	//启动wifi
	ESP_ERROR_CHECK(esp_wifi_start());
	//esp_wifi_connect();

    //wifi状态机开始运转
}



