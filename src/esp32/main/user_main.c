
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include <time.h>
#include <sys/time.h>
#include "wifi_setup.h"
#include "uart_handler.h"
#include "oven_one.h"





// create a rtos task to read temp
void app_main()
{
	
    nvs_flash_init();
    init_oven();
    init_wifisetup();
    uart_init(); 
}

