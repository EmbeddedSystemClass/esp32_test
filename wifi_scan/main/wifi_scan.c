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


static const char *TAG = "WIFI_SCAN";
#define WIFI_SSID   "siyue1L"

static EventGroupHandle_t wifi_event_group;  //定义事件的句柄
const int SCAN_DONE_BIT = BIT0;   //定义事件，占用事件变量的第0位，最多可以定义32个事件
static wifi_scan_config_t scanConf = {
	.ssid = NULL,
	.bssid = NULL,
	.channel = 0,
	.show_hidden = 1
}; //定义scanConf结构体，供函数esp_wifi_scan_start调用

esp_err_t event_handler(void *ctx,system_event_t *event)
{
	if(event->event_id == SYSTEM_EVENT_SCAN_DONE){
		xEventGroupSetBits(wifi_event_group,SCAN_DONE_BIT);  //设置事件位
	}
	return ESP_OK;
}

static void initialise_wifi(void)  //
{
	wifi_event_group = xEventGroupCreate();   //创建一个事件标志位
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler,NULL));  //创建事件的任务
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  //设置默认的wifi参数
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));  //初始化WIFI Alloc 资源为WIFI驱动，
	                                       //如wifi控制结构，TX/RX缓冲区，wifi NVS结构，此WIFI也启动wifi任务
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));  //设置wifi API 配置storge类型
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  //设置wifi 为station 模式
	ESP_ERROR_CHECK(esp_wifi_start());


}

static void scan_task(void *pvParameters)
{
	while(1){
		xEventGroupWaitBits(wifi_event_group,SCAN_DONE_BIT,0,1,portMAX_DELAY); //等待事件被置位，即等待扫描完成
		ESP_LOGI(TAG,"WiFi scan doen");
		xEventGroupClearBits(wifi_event_group,SCAN_DONE_BIT); //清除事件标志位

		uint16_t apCount = 0;
		esp_wifi_scan_get_ap_num(&apCount);  //get 扫到的AP 数量
		printf("Number of access points found:%d\n",apCount);
		if(apCount == 0){
			ESP_LOGI(TAG,"Nothing AP found");
			return;
		}

		wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t)*apCount);  //定义一个wifi_ap_record_t 的结构体链表空间
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount,list)); //获取上次扫描中找到的AP 列表。

		int i;
		printf("===============================================================\n");
		printf("            SSID           |  RSSI  |         AUTH            \n");
		printf("===============================================================\n");

		for(i=0;i<apCount;i++){
			char *authmode;
			switch(list[i].authmode){
				case WIFI_AUTH_OPEN:
			    	authmode = "WIFI_AUTH_OPEN";
				    break;
			    case WIFI_AUTH_WEP:
				    authmode = "WIFI_AUTH_WEP";
					break;
				case WIFI_AUTH_WPA_PSK:
				    authmode = "WIFI_AUTH_WPA_PSK";
					break;
				case WIFI_AUTH_WPA2_PSK:
				    authmode = "WIFI_AUTH_WPA2_PSK";
					break;
				case WIFI_AUTH_WPA_WPA2_PSK:
				    authmode = "WIFI_AUTH_WPA_WPA2_PSK";
					break;
                default:
				    authmode = "Unknown";
					break;
			}
			if(0 == strcmp((char *)list[i].ssid,WIFI_SSID))
			    printf("%26.26s |  %4d  |  %22.22s\n",list[i].ssid,list[i].rssi,authmode);
		}
		free(list);
		printf("\n\n");

		//scan again
		vTaskDelay(2000/portTICK_PERIOD_MS);   //调用延时函数重新扫描
		ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf,1));  //扫描所有可用的AP
	}
}

void app_main()  
{
	//初始化nvs
	ESP_ERROR_CHECK(nvs_flash_init());

	//初始化tcp/ip适配层
	tcpip_adapter_init();
    
	//初始化wifi
    initialise_wifi();

    xTaskCreate(&scan_task,"scan_task",2048,NULL,15,NULL);  //创建扫描任务

	ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf,1));

	return 0;
}



