#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define WIFI_SSID       "siyue1L"
#define WIFI_PASSWORD   "siyuexiongdi"

//#define EXAMPLE_ESP_WIFI_MODE_AP    CONFIG_ESP_WIFI_MODE_AP


static const char *TAG = "WIFI_STA";

static EventGroupHandle_t wifi_event_group_sta;  //定义事件的句柄
//static EventGroupHandle_t wifi_event_group_scan;
const int CONNECT_BIT = BIT0;   //定义事件，占用事件变量的第0位，最多可以定义32个事件

esp_err_t event_handler(void *ctx,system_event_t *event)
{
	switch(event->event_id){
		case SYSTEM_EVENT_STA_START:
		    esp_wifi_connect();
			break;
	    case SYSTEM_EVENT_STA_GOT_IP:
		    xEventGroupSetBits(wifi_event_group_sta,CONNECT_BIT);
			ESP_LOGI(TAG,"got ip:%s",ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			break;

		case SYSTEM_EVENT_AP_STACONNECTED:
		    ESP_LOGI(TAG,"station:"MACSTR" join,AID=%d",MAC2STR(event->event_info.sta_connected.mac),event->event_info.sta_connected.aid);
			break;

		case SYSTEM_EVENT_AP_STADISCONNECTED:
		    ESP_LOGI(TAG,"station:"MACSTR" leave,AID=%d",MAC2STR(event->event_info.sta_disconnected.mac),event->event_info.sta_disconnected.aid);
			break;
			
		case SYSTEM_EVENT_STA_DISCONNECTED:
		    esp_wifi_connect();
			xEventGroupClearBits(wifi_event_group_sta,CONNECT_BIT);
			break;

		default:
		    break;
	}
	return ESP_OK;
}

static void wifi_init_sta(void)
{
	wifi_event_group_sta = xEventGroupCreate();   //创建一个事件标志位
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler,NULL));  //创建事件的任务
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  //设置默认的wifi参数
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));  //初始化WIFI Alloc 资源为WIFI驱动，
	                                       //如wifi控制结构，TX/RX缓冲区，wifi NVS结构，此WIFI也启动wifi任务
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));  //设置wifi API 配置storge类型
	static wifi_config_t wifiConf = {
	    .sta = {
		    .ssid = WIFI_SSID,
		    .password = WIFI_PASSWORD
	    },
    }; //定义wifiConf结构体，设置连接AP 的SSID和PASSWORD 
	
	
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  //设置wifi 为station 模式
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wifiConf));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG,"wifi_init_sta finished.");
	ESP_LOGI(TAG,"connect to ap SSID：%s",WIFI_SSID);
}

static void got_ap_rssi(void)
{
	wifi_ap_record_t ap_info;
	while(1){

		esp_wifi_sta_get_ap_info(&ap_info);
		ESP_LOGI(TAG,"ap rssi:%d",ap_info.rssi);
		vTaskDelay(2000/portTICK_PERIOD_MS+1);
	}
}
void app_main()  
{
	//初始化nvs
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
	   ESP_ERROR_CHECK(nvs_flash_erase());
	   ret = nvs_flash_init();
	}
    ESP_ERROR_CHECK(ret);

	//初始化tcp/ip适配层
	tcpip_adapter_init();
    
	//初始化wifi
    wifi_init_sta();

    got_ap_rssi();

	return;
}



