#include "driver/uart.h"
#include "soc/uart_struct.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "uart_handler.h"
#include "pwm_control.h"

static int uart_num = UART_NUM_0;
QueueHandle_t uart0_queue;

static const char *TAG = "UART";

#define BUF_SIZE (1024)
void uart_task(void *pvParameters)
{
    int uart_num = (int) pvParameters;
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            //ESP_LOGI(TAG, "uart[%d] event:", uart_num);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.
                in this example, we don't process data in event, but read data outside.*/
                case UART_DATA:
                    uart_get_buffered_data_len(uart_num, &buffered_size);
                    //ESP_LOGI(TAG, "data, len: %d; buffered len: %d\n", event.size, buffered_size);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow\n");
                    //If fifo overflow happened, you should consider adding flow control for your application.
                    //We can read data out out the buffer, or directly flush the rx buffer.
                    uart_flush(uart_num);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full\n");
                    //If buffer full happened, you should consider encreasing your buffer size
                    //We can read data out out the buffer, or directly flush the rx buffer.
                    uart_flush(uart_num);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break\n");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error\n");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error\n");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    ESP_LOGI(TAG, "uart pattern detected\n");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d\n", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void uart_init(void)
{
     uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
       .rx_flow_ctrl_thresh = 122,
    };
    //Set UART parameters
    uart_param_config(uart_num, &uart_config);
    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Install UART driver, and get the queue.
    uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue, 0);
    //Set uart pattern detect function.
    uart_enable_pattern_det_intr(uart_num, '+', 3, 10000, 10, 10);
    //Create a task to handler UART event from ISR
    xTaskCreate(uart_task, "uart_task", 2048, (void*)uart_num, 2, NULL);
    //process data
    //uint8_t* data = (uint8_t*) malloc(2);
    uint8_t data[1];
    do {
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, 100 / portTICK_RATE_MS);
        if(len > 0) {
            ESP_LOGI(TAG, "uart read : len: %d data: %c\n", len, data[0]);
	    //TODO: find a better way to recieve commands from uart, hint:pointer to oven method param.
	    cmd_pwm(data[0], len);
        }
    } while(1);
}


