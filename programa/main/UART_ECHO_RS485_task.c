#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"

#include "libraries/UART_ECHO_RS485.h"
#include "config.h"
#include "main.h"

#define MODULE_NAME "UART_ECHO_RS485_MOD"

extern xQueueHandle http_server_uart_rs485_queue;

char imp_reg_temperature[8] = {0X01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x0A};      //Trama MODBUS para solicitar la lectura discreta del registro de la temperatura
char imp_reg_humidity[8] = {0X01, 0x04, 0x00, 0x02, 0x00, 0x01, 0x90, 0x0A};         //Trama MODBUS para solicitar la lectura discreta del registro de la humedad

//Receive UART_RS-485 messages and send them to UART_RS-485 queue
void uart_echo_rs485_task(void *pvParameters){

	char * in_message = (char *)malloc(LEN_MESSAGES_LORA);

	if(in_message == NULL){
		ESP_LOGE(MODULE_NAME, "%s malloc.1 failed\n", __func__);
	}

    const int uart_num = ECHO_UART_PORT;
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };

    // Set UART log level
    esp_log_level_set(MODULE_NAME, ESP_LOG_INFO);

    ESP_LOGI(MODULE_NAME, "Start RS485 application test and configure UART.");

    // Install UART driver (we don't need an event queue here)
    ESP_ERROR_CHECK(uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_LOGI(MODULE_NAME, "UART set pins, mode and install driver.");

    // Set UART pins as per KConfig settings
    ESP_ERROR_CHECK(uart_set_pin(uart_num, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Set RS485 half duplex mode
    ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));

    // Set read timeout of UART TOUT feature
    ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, ECHO_READ_TOUT));

    //Configuracion del sensor mediante el protocolo general que incluye el dispositivo en su hoja de descripcion
    ESP_LOGI(MODULE_NAME, "1");
    echo_send(uart_num, "TC:2.0", sizeof("TC:2.0"));
    ESP_LOGI(MODULE_NAME, "2");
    echo_send(uart_num, "HC:2.0", sizeof("HC:2.0"));
    ESP_LOGI(MODULE_NAME, "3");
    echo_send(uart_num, "BR:9600", sizeof("BR:9600"));
    ESP_LOGI(MODULE_NAME, "4");
    echo_send(uart_num, "HZ:2", sizeof("HZ:2"));

    // Allocate buffers for UART
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);

    ESP_LOGI(MODULE_NAME, "UART start recieve loop.\r\n");
    echo_send(uart_num, "Start RS485 UART test.\r\n", 24);

    BaseType_t uart_echo_rs485_status;

    // Task loop
    while(1) {

        //Read data from UART
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, PACKET_READ_TICS);

        //Write data back to UART
        if (len > 0) {

            echo_send(uart_num, "\r\n", 2);

            char prefix[] = "RS485 Received: [";
            ESP_LOGI(MODULE_NAME, "Received %u bytes:", len);
            printf("[ ");
            for (int i = 0; i < len; i++) {
                printf("0x%.2X ", (uint8_t)data[i]);

                echo_send(uart_num, (const char*)&data[i], 1);

                // Add a Newline character if you get a return charater from paste (Paste tests multibyte receipt/buffer)
                if (data[i] == '\r') {

                    echo_send(uart_num, "\n", 1);
                
                }

                *in_message = *data;
            }

            printf("] \n");
            echo_send(uart_num, "]\r\n", 3);
            
            // Respuesta del sensor con el valor de temperatura
            if(len == 1){

                int temp_value = (int)hexadecimalADecimal((char*)data, sizeof((char*)data));
                ESP_LOGI(MODULE_NAME, "\nTemperature measure %d", temp_value);
                in_message = (char*)temp_value; 
                
            }

            // SEND THE RECEIVED UART MESSAGE BY THE QUEUE
            uart_echo_rs485_status = xQueueSend(http_server_uart_rs485_queue, (void * ) &in_message, 10/portTICK_PERIOD_MS);

            // Check the message has been correctly send into the queue
            if(uart_echo_rs485_status == pdPASS){
                ESP_LOGI(MODULE_NAME, "UART message send to the queue correctly");
            }else{
                ESP_LOGI(MODULE_NAME, "ERROR SENDING MESSAGE");
            }

        } else {
            
            // Solicita constantemente la lectura discreta del registro de temperatura
            echo_send(uart_num, imp_reg_temperature, sizeof(imp_reg_temperature));
            ESP_ERROR_CHECK(uart_wait_tx_done(uart_num, 50));
            delay(100);
        }

        // Task delay
		vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    uart_driver_delete(uart_num);
    vTaskDelete(NULL);
}