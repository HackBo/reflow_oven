#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include "oven_one.h"
#include "driver/uart.h"

static xTaskHandle t_profile;
static xTaskHandle t_control;
static double proportion;
static double k_proportional = 1.5;
static double k_integral =  0.7;
static double time_window =0.2;
static double zone = 5.0;
static int time_start;
static double past_error;
static bool interrupt=false;


int now(void)
{
    struct timeval tv;
    int sec;
    gettimeofday(&tv, NULL);
    //sec = tv.tv_usec; 
    sec = tv.tv_sec; 
    return sec;
}



void init_oven(void)
{
    max_init();
    gpio_pad_select_gpio(OVEN_GPIO);
    gpio_set_direction(OVEN_GPIO, GPIO_MODE_OUTPUT);
}
void print_point(double temp, double time){
    static char buf[15];
    snprintf(buf,sizeof(buf), "%.2f %.2f\n", time,temp);
    //snprintf(buf,sizeof(buf), "%.2f %.2f\n\r", (float)t_tick,temp_0);
    uart_write_bytes(UART_NUM_0,buf,15);
}
void profile_power(void *pvParameters)
{
   time_start=now();
   int t_tick=-1;
   while ((now() - time_start) <= 400 ){
       int tmp_t=now(); 
       if(abs(t_tick - tmp_t) > 1){
       //if(t_tick != tmp_t){
           t_tick = now();
           double temp_0 = read_temp();
	   print_point(temp_0,(double)t_tick);
       }

   }
    vTaskDelete(t_profile);
}
void follow_curve(void *pvParameters)
{
    time_start = now();
    past_error=0;
    double time_before = temp_data[0][0];
    double temp_before = temp_data[0][1];
    proportion=0.0;
    for (int i = 0; i < 34 ; i++){
        aim_for(temp_before, time_before,
                     temp_data[i][1], temp_data[i][0]);
        time_before = temp_data[i][0];
        temp_before = temp_data[i][1];
	
    }
    vTaskDelete(t_control);

}
void aim_for(double temp_from, double time_from, double temp_to, double time_to)
{	
    //# Compute rect for temperature increase.
    double slope = (temp_to - temp_from) / (time_to - time_from);
    double K = temp_to - slope * time_to;
    //# This will take about 0.25s.
    double temp_0 = read_temp();
    ESP_LOGE(TAG,"time_to!  ->: %.2lf", time_to);
    while (((now() - time_start) <= time_to) ){
	   
        double time_in_curve = now() - time_start;
	ESP_LOGI(TAG,"time in curve: %.2lf", time_in_curve);
        double temp_wanted = slope * time_in_curve + K;
        double error = temp_wanted - temp_0;
	past_error += (error * time_window);
	ESP_LOGI(TAG,"wanted:%.2lf  to:%.2lf error %.2lf acc error %.2lf", temp_wanted, temp_to, error,past_error);
        if (error < 0){
            proportion = 0.0;
        }
        else if (error > zone){
            proportion = 1.0;
        }
        else {
            double integral = k_integral * past_error;
            double proportional = (k_proportional * error) / zone;
	    ESP_LOGI(TAG,"I:%.2lf P:%.2lf P+I: %.2lf ", integral, proportional,integral +proportional);
	    proportion =MAX(0.0, MIN(1.0, integral + proportional));
            ESP_LOGI(TAG, "time:%.2lf temp:%.2lf  target:%.2lf  error:%.2lf",time_in_curve, temp_0, temp_wanted, error);
	    print_point(temp_0, time_in_curve);
	} 
        ESP_LOGE(TAG, "proportion:%.2lf",proportion);
        //# Only turn on if we need to do some control.
        control_oven_with_bool(proportion > 0.0);
         //# Time reading from the thermocouple.
        double time_thermo = now();
        temp_0 = read_temp();
	//print_point(temp_0, time_thermo);
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
    	command_oven(1); 
                //follow_curve();
        if(!interrupt)
            xTaskCreate(follow_curve, "follow_curve_task", 2048, NULL, 10, &t_control);
//     	    xTaskCreate(profile_power, "profile", 2048, NULL, 10, &t_profile);

     }  
     else if (cmd == '*'){
    	double t= read_temp();
	ESP_LOGI(TAG,"TEMP %.2lf", t);
     }else if (cmd == 's'){
    	command_oven(0); 
        //vTaskDelete(t_profile);
        vTaskDelete(t_control);
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



