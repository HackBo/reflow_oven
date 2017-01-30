# Reflow Oven

Firmware and docs for new reflow oven.

Project codename: Oven One :-)

## Installing

In recent Ubuntu versions you'll need the following packages:

* python3
* python3-serial
* arduino

## Thermocouple

We're using a WZP PT100. There is a
[table for it](http://www.micropik.com/PDF/pt100.pdf)
but you can summarize them with a quadratic equation.

| Temperature   | Resistance    |
| ------------- |:-------------:|
| 0             | 100           |
| 100           | 138.51        |
| 200           | 175.86        |
| 300           | 212.05        |


    Temperature =  0.001113 * r * r + 2.329774 * r - 244.0946281

A line wasn't enough to summarize the table.

### Getting a voltage from the thermocouple

This [StackOverflow answer](http://stackoverflow.com/a/35302628/224184) was useful.

Using the following equation you can get one that gives you a voltage for different thermocouple resistance values.

![Equation for resistances](images/r1r2vinvout.png "Equation for resistances")

This is a [voltage divider](https://learn.sparkfun.com/tutorials/voltage-dividers).

    Vout =  (Vin * R2) / (R1 + R2)

    R2 = (Vout * R1) / (Vin - Vout)

We'll use Vin = 5v and we know R1's value, so the only variable is the thermocouple (R2).

For now we're using R1 = 221 ohms (what we measured).

## Arduino Uno pinout

### Analog inputs

- A0 : ADC input from the Vout formula above (Measuring Vout).

### Digital outputs

- Pin 3 : Output port 0 (resistance)
- Pin 4 : Output port 1 (resistance)
- Pin 13 (built-in led): Fast blink for error condition (invalid command, for instance).

## Protocol

Messages that are sent to the Arduino. Each command describes what is sent and what is sent back. All communications are in bytes.

- 0 : Select first port. Returns '0' (byte).
- 1 : Select second port. Returns '1' (byte).
- + : Turns on selected port. Returns '+'.
- - : Turns off selected port. Returns '-'.
- A : Asks for ADV value of selected port. Returns 'A', and then the ADC value in two bytes (first one is the highest one).

You can select 0 and 1 to do pings to the device. Recommended after startup.
