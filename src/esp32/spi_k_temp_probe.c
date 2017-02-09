#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"


#define PIN_NUM_MISO 25
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   21


static const char *TAG = "Spi_Thermocouple";

int spi_read() { 
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
void max_init(void) {
    //MAX6675 init 
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_MISO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
}


void read_temp_task(void *pvParameter){
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
        ESP_LOGI(TAG,"Celsius BINARY: %d",v);
	double celsius=v*0.25f;
        ESP_LOGI(TAG,"Celsius FLOAT : %.2lf", celsius);
    }
}

// create a rtos task to read temp
void app_main()
{
    nvs_flash_init();
    xTaskCreate(&read_temp_task, "read_temp_task", 2048, NULL, 5, NULL);
}
