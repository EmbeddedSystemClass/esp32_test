#include <stdio.h>
#include "string.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wpa2.h"
#include "esp_types.h"
#include "esp_smartconfig.h"

#include "nvs_flash.h"

#include "tcpip_adapter.h"

#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/ledc.h"
//#include "driver/uart.h"
//#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"


#define false      0
#define true       1
#define errno      (*__errno())

static const char *TAG = "sc";
static const char *TAG1 = "u_event";

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

//wifi连接成功事件
static EventGroupHandle_t wifi_event_group;

void smartconfig_example_task(void *parm);


//wifi状态机回调函数
static esp_err_t event_handler(void *ctx,system_event_t *event)
{
	switch(event->event_id){
		case SYSTEM_EVENT_STA_START:   //STA 开始工作
			ESP_LOGI(TAG1,"SYSTEM_EVENT_STA_START");
			//创建smartconfig任务
			xTaskCreate(smartconfig_example_task,"smartconfig_example_task",4096,NULL,3,NULL);
			break;

		case SYSTEM_EVENT_STA_GOT_IP:  //获取IP
			ESP_LOGI(TAG1,"SYSTEM_EVENT_STA_GOT_IP.");
			//sta链接成功，set事件组
			xEventGroupSetBits(wifi_event_group,CONNECTED_BIT);
			break;

		case SYSTEM_EVENT_STA_DISCONNECTED:   //断线
			ESP_LOGI(TAG1,"SYSTEM_EVENT_STA_DISCONNECTED");
			//断线重连
			esp_wifi_connect();
			xEventGroupClearBits(wifi_event_group,CONNECTED_BIT);
			break;

		default:
			break;
	}

	return ESP_OK;
}

//smartconfig 事件回调
static void sc_callback(smartconfig_status_t status,void *pdata)
{
	switch (status){
        case SC_STATUS_WAIT:   //等待配网
		    ESP_LOGI(TAG,"SC_STATUS_WAIT");
		    break;

	    case SC_STATUS_FIND_CHANNEL:   //扫描信道
		    ESP_LOGI(TAG,"SC_STATUS_FIND_CHANNEL");
			break;

		case SC_STATUS_GETTING_SSID_PSWD:   //获取到ssid 和 密码
		    ESP_LOGI(TAG,"SC_STATUS_GETTING_SSID_PSWD");
			break;

		case SC_STATUS_LINK:    //连接获取到的ssid和密码
		    ESP_LOGI(TAG,"SC_STATUS_LINK");
			wifi_config_t *wifi_config = pdata;
            
			//打印账号和密码
			ESP_LOGI(TAG,"SSID:%s",wifi_config->sta.ssid);
			ESP_LOGI(TAG,"PASSWORD:%s",wifi_config->sta.password);

            //断开默认的
			ESP_ERROR_CHECK(esp_wifi_disconnect());

            //设置获取的ap 和密码到寄存器
			ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA,wifi_config));
            
			//连接获取的ssid和密码
			ESP_ERROR_CHECK(esp_wifi_connect());

		case SC_STATUS_LINK_OVER:   //连接上配置后，结束
		    ESP_LOGI(TAG,"SC_STATUS_LINK_OVER");

			if(pdata != NULL){
				uint8_t phone_ip[4] = {0};
				memcpy(phone_ip,(uint8_t *)pdata,4);
				ESP_LOGI(TAG,"Phone ip:%d.%d.%d,%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
			}
            
			//发送sc结束事件
			xEventGroupSetBits(wifi_event_group,ESPTOUCH_DONE_BIT);
			break;

		default:
			break;
	}
}

//smartconfig任务
void smartconfig_example_task(void *parm)
{
	EventBits_t uxBits;

	//使用ESP-TOUCH配置工具
	ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
	
	//开始sc,注册回调
	ESP_ERROR_CHECK(esp_smartconfig_start(sc_callback));

	while(1){

		//死等事件组：CONNECTED_BIT | ESPTOUCH_DONE_BIT
		uxBits = xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT|ESPTOUCH_DONE_BIT,true,false,portMAX_DELAY);
        
		//sc结束
		if(uxBits & ESPTOUCH_DONE_BIT){
			ESP_LOGI(TAG,"smartconfig over");
			esp_smartconfig_stop();
            //此处smartconfig结束，并且连上设置的AP ，以下可以展开自定义工作了

			vTaskDelete(NULL);
		}
        
		//连上ap
		if(uxBits & CONNECTED_BIT){
			ESP_LOGI(TAG,"WiFi Connected to ap");
		}
	}
}

void app_main()
{
	//初始化nvs
	ESP_ERROR_CHECK(nvs_flash_init());
	tcpip_adapter_init();
    
	//事件组
	wifi_event_group = xEventGroupCreate();
    
	//注册wifi事件
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler,NULL));

    //wifi设置：默认设置，等待sc配置
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	
	//sta模式
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    //启动wifi
	ESP_ERROR_CHECK(esp_wifi_start());
}



