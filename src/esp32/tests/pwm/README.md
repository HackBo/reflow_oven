# LEDC(LED control - UART Driven PWM) Example

###This example shows:

 * init LEDC module:
 
     a.	You need to set the timer of LEDC first, this decide the frequency and resolution of PWM.
     
     b.	You need to set the LEDC channel you want to use, and bind the channel with one of the timers.
     
 * You can statically controll a pin's pwm output.
 
 * This example uses GPIO19 as LEDC ouput, and it will vary a led's brightness based on uart chars received ('+' and '-').

