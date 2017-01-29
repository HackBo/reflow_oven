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
