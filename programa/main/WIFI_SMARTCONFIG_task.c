#include "string.h"
#include "stdlib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_smartconfig.h"

#include "libraries/WIFI_SMARTCONFIG.h"
#include "config.h"
#include "main.h"

#define MODULE_NAME "WIFI_SMARTCONFIG_TASK"
int wifi_initiated = 0;

//extern xQueueHandle wifi_smartconfig_queue;

//Initialise WIFI SMARTCONFIG and, if it connects 
void wifi_smartconfig_task(void *pvParameters)
{
    //Init WIFI SMARCONFIG 
    ESP_ERROR_CHECK(nvs_flash_init());
    // if(wifi_initiated == 0){
    ESP_LOGI(MODULE_NAME, "Initialise WIFI SMARTCONFIG");
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
    ESP_LOGI(MODULE_NAME, "WIFI initiated");
    wifi_initiated = 1;
    // }else{
    //     ESP_ERROR_CHECK(esp_wifi_restore());
    //     initialise_wifi();
    // }
    

    // Task loop
    for (;;) {
        
        //Init WIFI SMARCONFIG 
        // ESP_LOGI(MODULE_NAME, "Initialise WIFI SMARTCONFIG");
        // initialise_wifi();
        //ESP_LOGI(MODULE_NAME, "WIFI SMARTCONFIG initiated");

        /*
        // Start the smartconfig for the first time after initialize wifi
        if (enable_wifi == true && enable_smartconfig == false){
            wifi_smartconfig_func(smartconfig_param); 
            ESP_LOGI(MODULE_NAME, "Start WIFI SMARTCONFIG for the first time\n");
            enable_smartconfig = true;
            ESP_LOGI(MODULE_NAME, "SMARTCONFIG initiated");
        }
	    
        BaseType_t wifi_smartconfig_status;
        
        
       
            // SEND THE RECEIVED MESSAGE BY THE QUEUE
            wifi_smartconfig_status = xQueueSend(wifi_smartconfig_queue, (void * ) &in_message, 10/portTICK_PERIOD_MS);

            // Check the message has been correctly send into the queue
            if(wifi_smartconfig_status == pdPASS){
                ESP_LOGI(MODULE_NAME, "WIFI message send to the queue correctly");
            }else{
                ESP_LOGI(MODULE_NAME, "ERROR SENDING MESSAGE");
            }

        */

        // Task delay
		vTaskDelay(1000 / portTICK_PERIOD_MS);

    }

    vTaskDelete(NULL);
}
