#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"

#include "config.h"
#include "main.h"
#include "libraries/OLED.h"

#define MODULE_NAME "OLED_DISP_MOD"

extern xQueueHandle display_queue;

uint8_t _color = WHITE;

void display_task(void *pvParameters)
{

    char buffer_message[LEN_MESSAGES_OLED];
    char * in_message ;

    // INIT Display
    initOLED( 128, 64, 21, 22, 16 );
	setFont(ArialMT_Plain_16 );
	clear();
	sendData();

    //OLED Send message screeen
	clear();
	drawString(0, 12, "Starting...", _color );
	sendDataBack();

    // Infinite Loop
    for (;;) {

        //Waiting to receive an incoming message.
        if (xQueueReceive(display_queue, (void * )&in_message, (portTickType)portMAX_DELAY)) {

            // Copy message to local buffer
            memcpy(buffer_message, in_message, LEN_MESSAGES_OLED);

            // Send to screen
            clear();
            drawString(0, 12, buffer_message, _color );
            sendDataBack();

        }

        // Task delay
		vTaskDelay(10 / portTICK_PERIOD_MS);

    }

    vTaskDelete(NULL);
}