# Reflow Owen

Firmware and docs for new reflow oven 

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


    Resistance =  0.001113 * x * x + 2.329774 * x - 244.0946281

A line wasn't enough to summarize the table.

### Getting a voltage from the thermocouple

This [stackoverflow answer](http://stackoverflow.com/a/35302628/224184) was useful.

Using the following equation you can get one that gives you a voltage for different thermocouple resistance values.
![Equation for resistances](images/images/r1r2vinvout.png "Equation for resistances")

    Vout =  (Vin * R2) / (R1 + R2)

We'll use Vin = 5v and we know R1's value, so the only variable is the thermocouple (R2).
