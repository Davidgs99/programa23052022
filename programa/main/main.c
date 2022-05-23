// Libraries Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"

// Project Includes
#include "main.h"
#include "config.h"
//#include "MODBUS_RECEIVER_task.h"
#include "LORA_RECEIVER_task.h"
#include "WIFI_SMARTCONFIG_task.h"
//#include "libraries/WIFI_STATION.h"
#include "libraries/WIFI_SMARTCONFIG.h"
//#include "libraries/MODBUS_MASTER.h"
#include "libraries/LORA.h"
#include "HTTP_SERVER_task.h"
#include "OLED_DISPLAY_task.h"
#include "libraries/UART_ECHO_RS485.h"
#include "UART_ECHO_RS485_task.h"

#define APP_NAME "TFG_APP"

// FREE RTOS DEFINITIONS
xQueueHandle display_queue = NULL;
xQueueHandle uart_echo_rs485_queue = NULL;
xQueueHandle lora_receiver_queue = NULL;
//xQueueHandle wifi_smartconfig_queue = NULL;
xQueueHandle http_server_lora_queue = NULL;
xQueueHandle http_server_uart_rs485_queue = NULL;

/*
 *  MAIN VARIABLES
 ****************************************************************************************
 */

//Declaration of communications counter
int comm_counter = 0;

void delay( int msec ) {
    vTaskDelay( msec / portTICK_PERIOD_MS);
}

/*
 *  INIT BOARD FUNCTIONS
 ****************************************************************************************
 */

//Configure GPIO ports
bool getConfigPin()
{
    gpio_config_t io_conf;
    gpio_num_t pin = (gpio_num_t) SENDER_RECEIVER_PIN;

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << pin );
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    gpio_config(&io_conf);

    return gpio_get_level( pin ) == 0;
}

/*
 *  MAIN TASK 
 ****************************************************************************************
 * Main task area. 
 */

//Control communications between system functions
void main_task( void* param ){
	
	// Initialize the default NVS partition. 
	//ESP_ERROR_CHECK(nvs_flash_init());
	
	// //Initialise Smartconfig
	// ESP_LOGI(APP_NAME, "Initialise WIFI");
	// initialise_wifi();
	// enable_wifi = true;
	// ESP_LOGI(APP_NAME, "WIFI initiated");
	// delay(10000);

	// //Stop Smartconfig
	// ESP_LOGI(APP_NAME, "Stop WIFI");
	// esp_smartconfig_stop();
	// enable_smartconfig = false;

	// Config Flash Pin
	gpio_num_t fp = (gpio_num_t) FLASH_PIN;
	gpio_pad_select_gpio( fp );
	gpio_set_direction( fp , GPIO_MODE_OUTPUT );

	//Asign the memory for the message in the heap (with the display message size)
	char * message = (char *)malloc(LEN_MESSAGES_SERVER);
	if(message == NULL){
		ESP_LOGE(APP_NAME, "%s malloc.1 failed\n", __func__);
	}

	for ( ;; )
	{	
		char *in_uart_rs485_message = NULL;					//Received MODBUS message
		char *in_lora_message = NULL;					//Received LORA message
		// char *in_wifi_message = NULL;					//Received WIFI message
		// char *in_http_server_message = NULL;			//Received HTTP server message
				
		//Waiting to receive an MODBUS incoming message.
		if (xQueueReceive(uart_echo_rs485_queue, (void * )&in_uart_rs485_message, (portTickType)portMAX_DELAY)) 
		{	
			ESP_LOGI(APP_NAME, "UART_ECHO_RS485 MODE"); 

			//Create object to send messages to queues
			BaseType_t uart_rs485_status_display;
			BaseType_t uart_rs485_status_server;

			// Genarate display message
			sprintf(message, "[%d]:%s", comm_counter, in_uart_rs485_message);	
						
			// SEND MESSAGE TO THE DISPLAY
			uart_rs485_status_display = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// SEND MESSAGE TO THE SERVER
			uart_rs485_status_server = xQueueSend(http_server_uart_rs485_queue, (void * ) &message, 10/portTICK_PERIOD_MS);
					
			// Check the the message has been correctly send into the display queue
			if(uart_rs485_status_display == pdPASS){
			ESP_LOGI(APP_NAME, "UART message send to display correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Check the the message has been correctly send into the MODBUS server queue
			if(uart_rs485_status_server == pdPASS){
			ESP_LOGI(APP_NAME, "UART message send to server correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Update counter
			comm_counter++;

		}

		//Waiting to receive an LORA incoming message.
		if (xQueueReceive(lora_receiver_queue, (void * )&in_lora_message, (portTickType)portMAX_DELAY)) 
		{
			ESP_LOGI(APP_NAME, "LORA MODE"); 

			//Create object to send messages to queues
			BaseType_t lora_status_displey;
			BaseType_t lora_status_server;

			// Genarate display message
			sprintf( message, "[%d]:%s", comm_counter, in_lora_message);	
						
			// SEND MESSAGE TO THE DISPLAY
			lora_status_displey = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// SEND MESSAGE TO THE SERVER
			lora_status_server = xQueueSend(http_server_lora_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// Check the the message has been correctly send into the queue
			if(lora_status_displey == pdPASS){
				ESP_LOGI(APP_NAME, "LORA message send to display correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Check the the message has been correctly send into the server queue
			if(lora_status_server == pdPASS){
			ESP_LOGI(APP_NAME, "LORA message send to server correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Update counter
			comm_counter++;
		
		}

		// Waiting to receive an WIFI incoming message.
		// if (xQueueReceive(wifi_smartconfig_queue, (void * )&in_wifi_message, (portTickType)portMAX_DELAY)) 
		// {
		// 	ESP_LOGI(APP_NAME, "WIFI MODE"); 

		// 	//Create object to send messages to queues
		// 	BaseType_t wifi_status_display;
		// 	//BaseType_t wifi_status_server;

		// 	// Genarate display message
		// 	sprintf( message, "[%d]:%s", comm_counter, in_wifi_message);	
						
		// 	// SEND MESSAGE TO THE DISPLAY
		// 	wifi_status_display = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

		// 	/*
		// 	// SEND MESSAGE TO THE SERVER
		// 	wifi_status_server = xQueueSend(http_server_queue, (void * ) &message, 10/portTICK_PERIOD_MS);		¿QUE LE ENVIO AL SERVIDOR DESDE EL WIF 21.03.22DI?
		// 	*/

		// 	// Check the the message has been correctly send into the queue
		// 	if(wifi_status_display == pdPASS){
		// 		ESP_LOGI(APP_NAME, "WIFI messages send to display correctly");
		// 	}else{
		// 		ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGEs");
		// 	}

			/*
			// Check the the message has been correctly send into the server queue
			if(wifi_status_server == pdPASS){
			ESP_LOGI(APP_NAME, "WIFI message send to server correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}
			*/

			// Update counter
		// 	comm_counter++;

		// }

		// Waiting to receive an HTTP SERVER incoming message from LORA.
		// if (xQueueReceive(http_server_queue, (void * )&in_http_server_message, (portTickType)portMAX_DELAY)) 
		// {
		// 	ESP_LOGI(APP_NAME, "HTTP SERVER MODE"); 
	
		// 	BaseType_t http_server_status;

		// 	//SEND MESSAGE TO THE DISPLAY
		// 	ESP_LOGI(APP_NAME, "Send received message from server queue TO DISPLAY QUEUE");
		// 	http_server_status = xQueueSend(display_queue, (void * ) &in_http_server_message, 10/portTICK_PERIOD_MS);

		// 	char * replica = in_http_server_message;
		// 	ESP_LOGI(APP_NAME, "mensaje replicado de la cola del servidor = %s", (char *)replica);

		// 	//SEND MESSAGE TO THE server again
		// 	ESP_LOGI(APP_NAME, "Send received message from server queue TO SERVER QUEUE");
		// 	http_server_status = xQueueSend(http_server_queue, (void * ) &replica, 10/portTICK_PERIOD_MS);

		// 	//Check the the message has been correctly send into the queue
		// 	if(http_server_status == pdPASS){
		// 		ESP_LOGI(APP_NAME, "Server message send correctly");
		// 	}else{
		// 	 	ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGEs");
		// 	}

		// 	// Update counter
		// 	comm_counter++;
			
		// }
		

		// Blink the led
		gpio_set_level( fp, 1);
		delay(100);

		gpio_set_level( fp, 0);
		delay(1000);

		// Task delay
		vTaskDelay(50 / portTICK_PERIOD_MS);

	}

}

/*
 *  MAIN
 ****************************************************************************************
 * Use to initial configuration of the system and starting the tasks.
 */

//Create queues and init tasks
void app_main()
{

	/*** Init the FREERTOS queques ***/
	
	// Create the queque for receiving UART Messages
    uart_echo_rs485_queue = xQueueCreate(5, sizeof(char *));
    if( uart_echo_rs485_queue == NULL )
	{ 
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the uart_echo_rs485_queue\n");

	}

	// Create the queque for receiving LORA Messages
    lora_receiver_queue = xQueueCreate(5, sizeof(char *));
    if( lora_receiver_queue == NULL )
	{ 
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the lora_receiver_queue\n");

	}

	// Create the queque for receiving WIFI Messages
    // wifi_smartconfig_queue = xQueueCreate(5, sizeof(char *));
    // if( wifi_smartconfig_queue == NULL )
	// { 
    //     // There was not enough heap memory space available to create the message buffer. 
    //     ESP_LOGE(APP_NAME, "Not enough memory to create the wifi_smartconfig_queue\n");

	// }

	// Create the queque for sending the LORA messages to HTTP server
    http_server_lora_queue = xQueueCreate(5, sizeof(char *));								
    if( http_server_lora_queue == NULL )
	{
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the http_server_lora_queue\n");

	}

	// Create the queque for sending the MODBUS messages to HTTP server
    http_server_uart_rs485_queue = xQueueCreate(5, sizeof(char *));								
    if( http_server_uart_rs485_queue == NULL )
	{
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the http_server_uart_rs485_queue\n");

	}

	// Create the queque for sending the messages to display
    display_queue = xQueueCreate(10, sizeof(char *));
    if( display_queue == NULL )
	{
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the display_queue\n");

	}


	/*** Init the system tasks ***/

	xTaskCreate(main_task, "main_task", 10000, NULL, 10, NULL);

	//xTaskCreate(uart_echo_rs485_task, "uart_echo_rs485_task", ECHO_TASK_STACK_SIZE, NULL, 8, NULL);

	//xTaskCreate(lora_receiver_task, "lora_receiver_task", 10000, NULL, 7, NULL);

	xTaskCreate(wifi_smartconfig_task, "wifi_smartconfig_task", 4096, NULL, 3, NULL);

	xTaskCreate(display_task, "display_task",5000, NULL, 1, NULL);

	xTaskCreate(http_server_task, "http_server_task", 10000, NULL, 2, NULL);
}

