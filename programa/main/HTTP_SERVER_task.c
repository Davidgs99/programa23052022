#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include "esp_http_server.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"

#include "libraries/HTTP_SERVER.h"
#include "config.h"
#include "main.h"

#define MODULE_NAME "HTTP_SERVER_TASK"
    
extern xQueueHandle http_server_lora_queue;
extern xQueueHandle http_server_uart_rs485_queue;

char *queue_lora_message;
char *queue_uart_rs485_message;

int server_initiated = 0;

httpd_handle_t server = NULL;               //Create a server which will show sensors measures
    
//Start server and upload messages 
void http_server_task(void *pvParameters){

    ESP_LOGI(MODULE_NAME, "Waiting to enable wifi connection to init web sever\n");

    // Task loop
    for (;;) {

        // Start the server for the first time
        if (enable_smartconfig == true && server_initiated == 0){
            server = start_webserver();  
            ESP_LOGI(MODULE_NAME, "Start web server for the first time\n");
            server_initiated = 1;
        }
        
        while(server_initiated == 1){
            
            ESP_LOGI(MODULE_NAME, "SERVER INITIATED\n");

            if (xQueueReceive(http_server_lora_queue, (void * )&queue_lora_message, (portTickType)portMAX_DELAY)){    //If receive any message from server queue, send it to server
                
                ESP_LOGI(MODULE_NAME, "LORA queue data = %s", (char *)queue_lora_message);
                
                sprintf(&server_string_lora, queue_lora_message);
                
                ESP_LOGI(MODULE_NAME, "Data message received from LORA queue and send to server successfully\n");

            }

            if (xQueueReceive(http_server_uart_rs485_queue, (void * )&queue_uart_rs485_message, (portTickType)portMAX_DELAY)){    //If receive any message from server queue, send it to server
                
                ESP_LOGI(MODULE_NAME, "MODBUS queue data = %s", (char *)queue_uart_rs485_message);
                
                sprintf(&server_string_modbus, queue_uart_rs485_message);
                
                ESP_LOGI(MODULE_NAME, "Data message received from MODBUS queue and send to server successfully\n");

            }

            break; 
        }

        if( enable_smartconfig == false && server_initiated == 1){
            // Stop the server 
            stop_webserver(server);
            ESP_LOGI(MODULE_NAME, "Stop web server\n");
            server_initiated = 0;
	    }

        // Task delay
		vTaskDelay(10 / portTICK_PERIOD_MS);

    }

    vTaskDelete(NULL);
}