#ifndef __OVEN_ONE_H_
#define __OVEN_ONE_H_

#include "max6675.h"
#define k_proportional 1.5
#define k_integral 0.15
#define time_window 0.5
#define zone 15
#define OVEN_GPIO 5
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)
xTaskHandle t_profile;
xTaskHandle t_control;

static double temp_data[34][2] = {
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

static const char *TAG = "REFLOW";
static int time_start;
static double past_error;
static bool interrupt=false;


void init_oven(void);
void follow_curve(void *pvParameters);
static void aim_for(double temp_from, double time_from, double temp_to, double time_to);
void cmd_oven(char  cmd);
void oven_sleep(int seconds);
void command_oven(int on);
void control_oven_with_bool(bool on);




#endif /* __OVEN_ONE_H__ */

