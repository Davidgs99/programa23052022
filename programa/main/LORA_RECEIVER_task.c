#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"

#include "libraries/LORA.h"
#include "config.h"
#include "main.h"

#define MODULE_NAME "LORA_RECEIVER_MOD"

extern xQueueHandle lora_receiver_queue;

int lora_initiated = 0;

void writeLoraRegisterMessage( char * message)
{

    ESP_LOGI(MODULE_NAME, "Sending message Lora: %s", message);

	loraBeginPacket(false);

	//lora->setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
	loraWrite( (uint8_t*) message, (size_t) strlen(message) );
	loraEndPacket(false);

}

//Receive Lora messages and send them to Lora queue
void lora_receiver_task(void *pvParameters)
{

    // Inits the Lora module
	ESP_LOGI(MODULE_NAME, "Init Lora");
    if(lora_initiated == 0){
        loraInit( PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, PIN_NUM_CS, RESET_PIN, PIN_NUM_DIO, 10 );
        lora_initiated = 1;
    }else if(lora_initiated > 0){
        // initializeReset(RESET_PIN);
        // initializeDIO(PIN_NUM_DIO);
        // initialize(10);
    }
    // Set the Lora reception mode
	loraReceive(0);
	ESP_LOGI(MODULE_NAME, "Lora initiated");

    //Asign the memory for the incomming message in the heap
	char * in_message = (char *)malloc(LEN_MESSAGES_LORA);
	if(in_message == NULL){
		ESP_LOGE(MODULE_NAME, "%s malloc.1 failed\n", __func__);
	}

    // Task loop
    for (;;) {

        // Check if we have recieved any message
        if(loraGetDataReceived()){

            BaseType_t lora_status;

            int packetSize = loraHandleDataReceived( in_message );

            loraSetDataReceived( false );

            // SEND THE RECEIVED MESSAGE BY THE QUEUE
            lora_status = xQueueSend(lora_receiver_queue, (void * ) &in_message, 10/portTICK_PERIOD_MS);

            // Check the message has been correctly send into the queue
            if(lora_status == pdPASS){
                ESP_LOGI(MODULE_NAME, "LORA message send to the queue correctly");
            }else{
                ESP_LOGI(MODULE_NAME, "ERROR SENDING MESSAGE");
            }

        }

        // Task delay
		vTaskDelay(10000 / portTICK_PERIOD_MS);
        //loraIdle();
    }

    vTaskDelete(NULL);
}
