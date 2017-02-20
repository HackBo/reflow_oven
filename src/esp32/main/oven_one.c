#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include "oven_one.h"

int now(void)
{
    struct timeval tv;
    int sec;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec; 
    return sec;
}



void init_oven(void)
{
    max_init();
    gpio_pad_select_gpio(OVEN_GPIO);
    gpio_set_direction(OVEN_GPIO, GPIO_MODE_OUTPUT);
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
void aim_for(double temp_from, double time_from, double temp_to, double time_to)
{	
    //# Compute rect for temperature increase.
    double slope = (temp_to - temp_from) / (time_to - time_from);
    double K = temp_to - slope * time_to;
    //# This will take about 0.25s.
    double temp_0 = read_temp();
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
	    proportion =MAX(0.0, MIN(1.0, integral + proportional));
            //proportion = MIN(1.0,  proportional);
            ESP_LOGI(TAG, "time:%.2lf temp:%.2lf  target:%.2lf  error:%.2lf  proportion:%.2lf"
                          ,time_in_curve, temp_0, temp_wanted, error, proportion);
	} 
        //# Only turn on if we need to do some control.
        control_oven_with_bool(proportion > 0.0);
         //# Time reading from the thermocouple.
        double time_thermo = now();
        temp_0 = read_temp();
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
    	double t= read_temp();
	ESP_LOGI(TAG,"TEMP %.2lf", t);
     }else if (cmd == 's'){
	interrupt=!interrupt;
     }



}

void oven_sleep(int seconds)
{
    vTaskDelay((seconds*1000)/portTICK_PERIOD_MS);
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





void control_oven_with_bool(bool on)
{
    if(on){
        cmd_oven('+');
    } else {
        cmd_oven('-');
    }
}



