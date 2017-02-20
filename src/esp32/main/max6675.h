#ifndef __MAX6675_H_
#define __MAX6675_H_
#define PIN_NUM_MISO 25
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   21

#include "driver/gpio.h"

double read_temp(void);
void max_init(void);
int spi_read(void);

#endif /* __MAX6675_H__ */
