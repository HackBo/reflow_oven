
#include "max6675.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"


void max_init(void) 
{
    //MAX6675 init 
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_MISO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
}

double read_temp(void)
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
