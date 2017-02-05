// this library is public domain. enjoy!
// www.ladyada.net/learn/sensors/thermocouple
// Modified by Nelson Castillo <nelsoneci@gmail.com> to return
// int instead of doubles.

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

class MAX6675 {
 public:
  MAX6675(int8_t SCLK, int8_t CS, int8_t MISO);
  // Temperature multiplied by four.
  long ReadCelsiusByFour(void);
 private:
  int8_t sclk, miso, cs;
  uint8_t spiread(void);
};
