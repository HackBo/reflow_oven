/* Pwm control based on Ledc fade example
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/xtensa_api.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "esp_attr.h"   
#include "esp_err.h"
#include "nvs_flash.h"
#include "uart_handler.h"
#include "pwm_control.h"

/* 
 * This example is meant to test the pwm output.
 * Next step is to use the PWM in a PID for a reflow oven.
 */

#define LEDC_IO_1    (18)

//Max PWM is two to the power of the timer bit size LEDC_TIMER
#define MAX_PWM	1024

static int current_PWM = 0;

// MUST BE STATIC!
static void set_PWM(int level)
{
    if(level <= 0){
	current_PWM = 0;
    } else if(level >= MAX_PWM){
        current_PWM = MAX_PWM;
    } else {
	current_PWM = level;
    }
    printf("LEDC set duty without fade %d\n", current_PWM);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, current_PWM);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
}

//Function called from UART input
void cmd_pwm(char cmd)
{
    if(cmd == '1'){
	set_PWM(current_PWM + 100);
    } else if (cmd == '0'){
	set_PWM(current_PWM - 100);
    }
}

void init_pwm() 
{   
    ledc_timer_config_t ledc_timer = {
        .bit_num = LEDC_TIMER_10_BIT,
        .freq_hz = 50,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = LEDC_IO_1,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel);

}
