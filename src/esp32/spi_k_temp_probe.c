
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "driver/uart.h"
#include "freertos/queue.h"
#include "soc/uart_struct.h"
#include <time.h>
#include <sys/time.h>
#define BUF_SIZE (1024)
#define OVEN_GPIO 5
#define PIN_NUM_MISO 25
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   21
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)
double read_oven_temp(void);
static void aim_for(double temp_from, double time_from, double temp_to, double time_to);

//void follow_curve(void);
void follow_curve(void *pvParameters);
double temp_data[34][2] = {
	{0.0  ,25},
{60.0 ,37},
{75.0 ,41},
{90.0 ,45.5626260115559},
{105.0 ,52.2666468302371},
{120.0 ,59.3091143565639},
{135.0 ,66.8040758965356},
{150.0 ,74.8655787561516},
{165.0 ,83.607670241411},
{180.0 ,93.1443976583133},
{195.0 ,103.585385650617},
{210.0 ,114.774899127665},
{225.0 ,126.145895410453},
{240.0 ,137.095950522054},
{255.0 ,147.022640485541},
{270.0 ,155.364188176892},
{285.0 ,162.031336137084},
{300.0 ,167.239678303866},
{315.0 ,171.209889471604},
{330.0 ,174.162644434662},
{345.0 ,176.318617987403},
{360.0 ,177.898484924191},
{375.0 ,179.12292003939},
{390.0 ,180.212598127366},
{405.0 ,181.38819398248},
{420.0 ,182.870382399098},
{435.0 ,184.879838171583},
{450.0 ,187.6372360943},
{465.0 ,191.360539752927},
{480.0 ,196.105040212017},
{495.0 ,201.673886128377},
{510.0 ,207.848536489333},
{560.0 ,215.14108649433},
{860.0 ,0.0}};
static int uart_num = UART_NUM_0;

static const char *TAG = "REFLOW";
static int time_start;
static double past_error;
static bool interrupt=false;
#define k_proportional 1.5
#define k_integral 0.15
#define time_window 0.5
#define zone 15

int now(void)
{
    struct timeval tv;
    int sec;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec; 
    return sec;
}

void oven_sleep(int seconds)
{
    vTaskDelay((seconds*1000)/portTICK_PERIOD_MS);
}

int spi_read(void) 
{ 
    int i;
    int d = 0;
    for (i=7; i>=0; i--){
	gpio_set_level(PIN_NUM_CLK, 0);
	vTaskDelay(10/portTICK_PERIOD_MS);
	if (gpio_get_level(PIN_NUM_MISO)){
	//set the bit to 0 no matter what
            d |= (1 << i);
	}	
	gpio_set_level(PIN_NUM_CLK, 1);
	vTaskDelay(10/portTICK_PERIOD_MS);

    }
    return d;
}

void command_oven(int on)
{
    if(on) {
        gpio_set_level(OVEN_GPIO, 1);
    }
    else {
        gpio_set_level(OVEN_GPIO, 0);
    }
}


void cmd_oven(char  cmd)
{
     if(cmd == '+'){
    	command_oven(1); 
     }
     else if (cmd == '-'){
    	command_oven(0); 
     }
     else if (cmd == '.'){
        time_start = now();
	past_error=0;
        //follow_curve();
        if(!interrupt)
       	    xTaskCreate(follow_curve, "follow_curve_task", 2048, NULL, 10, NULL);

     }  
     else if (cmd == '*'){
    	double t= read_oven_temp();
	ESP_LOGI(TAG,"TEMP %.2lf", t);
     }else if (cmd == 's'){
	interrupt=!interrupt;
     }



}


double read_oven_temp(void)
{
    int v=0;
    gpio_set_level(PIN_NUM_CS, 0);
    vTaskDelay(250/portTICK_PERIOD_MS);
    v = spi_read();
    v <<= 8;
    v |= spi_read();	
    gpio_set_level(PIN_NUM_CS, 1);
    vTaskDelay(250/portTICK_PERIOD_MS);
    if (v & 0x4) {
        // uh oh, no thermocouple attached!
	return 0.0;
    }
    v >>= 3;
    double celsius=v*0.25f;
    return celsius;
}

void control_oven_with_bool(bool on){
    if(on){
        cmd_oven('+');
    } else {
        cmd_oven('-');
    }
}

void write_log(double temp, double time)
{
    char buf[15];
    sprintf(buf,"%.2lf %.2lf",temp,time);
    uart_write_bytes(uart_num, (const char*)buf, 15);
}


static void aim_for(double temp_from, double time_from, double temp_to, double time_to)
{	
    //# Compute rect for temperature increase.
    double slope = (temp_to - temp_from) / (time_to - time_from);
    double K = temp_to - slope * time_to;
    //# This will take about 0.25s.
    double temp_0 = read_oven_temp();
    while (((now() - time_start) <= time_to) && !interrupt){
	   
        double time_in_curve = now() - time_start;
        double temp_wanted = slope * time_in_curve + K;
        double error = temp_wanted - temp_0;
	ESP_LOGI(TAG,"wanted:%.2lf vs to:%.2lf ", temp_wanted, temp_to);
	past_error += (error * time_window);
        double proportion=0.0;
        if (error < 0){
            proportion = 0.0;
        }
        else if (error > zone){
            proportion = 1.0;
        }
        else {
            double integral = k_integral * past_error;
            double proportional = (k_proportional * error) / zone;
	    ESP_LOGI(TAG,"I:%.2lf P:%.2lf P+I(proportion): %.2lf ", integral, proportional,integral +proportional);
	    proportion = MIN(1.0, integral + proportional);
            //proportion = MIN(1.0,  proportional);
            ESP_LOGI(TAG, "time:%.2lf temp:%.2lf  target:%.2lf  error:%.2lf  proportion:%.2lf"
                          ,time_in_curve, temp_0, temp_wanted, error, proportion);
	} 
        //# Only turn on if we need to do some control.
        control_oven_with_bool(proportion > 0.0);
         //# Time reading from the thermocouple.
        double time_thermo = now();
        temp_0 = read_oven_temp();
        time_thermo = now() - time_thermo;
        //# How long do we need to be on and off?
        double time_need_on = time_window * proportion - time_thermo;
        double time_need_off = time_window - time_need_on;
        oven_sleep(MAX(0, time_need_on));
        control_oven_with_bool(proportion >= 1.0);
        oven_sleep(MAX(0, time_need_off));
	//write_log(temp_0,time_in_curve);
    }
    
}

void follow_curve(void *pvParameters)
{
    double time_before = temp_data[0][0];
    double temp_before = temp_data[0][1];
    for (int i = 0; i < 34 && !interrupt ; i++){
         ESP_LOGI(TAG, "temp data time:%.2f temp: %.2f  \n", temp_data[i][0], temp_data[i][1]);
        //int cur_time = now();	
        //ESP_LOGI(TAG,"Celsius FLOAT : %.2lf time: %d\n", read_oven_temp(), cur_time);
        aim_for(temp_before, time_before,
                     temp_data[i][1], temp_data[i][0]);
        time_before = temp_data[i][0];
        temp_before = temp_data[i][1];
	
    }
    vTaskDelete(NULL);

}

QueueHandle_t uart0_queue;


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

void init_oven()
{
    gpio_pad_select_gpio(OVEN_GPIO);
    gpio_set_direction(OVEN_GPIO, GPIO_MODE_OUTPUT);
}

void uart_evt_test()
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
	    //follow_curve();
	    cmd_oven(data[0]);
            //uart_write_bytes(uart_num, (const char*)data, len);
        }
    } while(1);
}

void max_init(void) 
{
    //MAX6675 init 
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_MISO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
}

void read_temp_task(void *pvParameter)
{
    max_init();
    while(1) {
        int v=0;
    	gpio_set_level(PIN_NUM_CS, 0);
	vTaskDelay(250/portTICK_PERIOD_MS);
	v = spi_read();
	v <<= 8;
	v |= spi_read();
    	gpio_set_level(PIN_NUM_CS, 1);
    	vTaskDelay(250/portTICK_PERIOD_MS);
       	if (v & 0x4) {
	    // uh oh, no thermocouple attached!
	    ESP_LOGE(TAG,"im not attached dude");
	}
        v >>= 3;
	uint32_t cur_time= now();	
        ESP_LOGI(TAG,"Celsius BINARY: %d curtime : %d\n",v,(int)cur_time );
	double celsius=v*0.25f;
        ESP_LOGI(TAG,"Celsius FLOAT : %.2lf\n", celsius);
    }
}

// create a rtos task to read temp
void app_main()
{
	
    nvs_flash_init();
    init_oven();
    max_init();
    //xTaskCreate(&read_temp_task, "read_temp_task", 2048, NULL, 5, NULL);
    uart_evt_test();
    
}

