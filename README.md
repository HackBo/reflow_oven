# Reflow Oven

Firmware and docs for new reflow oven.

Project codename: Oven One :-)

## Installing

In recent ubuntu versions you'll need the following packages:

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


    Temperature =  0.001113 * t * t + 2.329774 * t - 244.0946281

A line wasn't enough to summarize the table.

### Getting a voltage from the thermocouple

This [stackoverflow answer](http://stackoverflow.com/a/35302628/224184) was useful.

Using the following equation you can get one that gives you a voltage for different thermocouple resistance values.

![Equation for resistances](images/r1r2vinvout.png "Equation for resistances")

    Vout =  (Vin * R2) / (R1 + R2)

We'll use Vin = 5v and we know R1's value, so the only variable is the thermocouple (R2).

## Arduino pinout

### Digital outputs

- Pin 3 : Output port 0 (resistance)
- Pin 4 : Output port 1 (resistance)
- Pin 13 (builtin led): ON for error condition (invalid command, for instance).

## Protocol

Messages that are sent to the arduino. Each command describes what is sent and what is sent back. All communications are in bytes.

- 0 : Select first port. Returns '0' (byte).
- 1 : Select second port. Returns '1' (byte).
- + : Turns on selected port. Returns '+'.
- - : Turns off selected port. Returns '-'.
- A : Asks for ADV value of selected port. Returns 'A', and then the ADC value in two bytes (first one is the highest one).

You can select 0 and 1 to do pings to the device. Recommended after startup.
