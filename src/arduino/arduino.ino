#include "max6675.h"

// Arduino Nano ATmega 328.
const int kPinPort0 = PD2;
const int kPinPort1 = PD3;
const int kPinErrorLed = PD4;
const int kTCS0 = PD6;
const int kTCS1 = PD7;
const int kMaxTemp = 240 * 4; // 240°

int current_thermo = 0;
MAX6675 thermocouple0(SCK, kTCS0, MISO);
MAX6675 thermocouple1(SCK, kTCS1, MISO);

void loop() {
  if (!Serial.available())
    return;
  const int cmd = Serial.read();
  Serial.write(cmd);
  switch (cmd) {
    char buffer[16];
    long temp;
    case '0': case '1':
      current_thermo = cmd - '0';
      break;
    case '+': case '-':
      digitalWrite(current_thermo ? kPinPort1 : kPinPort0, cmd == '+' ? HIGH : LOW);
      break;
    case 'T':
      temp = current_thermo ? thermocouple1.ReadCelsiusByFour() :
                              thermocouple0.ReadCelsiusByFour();
      if (temp == -1 || temp > kMaxTemp)
        goto error;
      sprintf(buffer, "%ld.", temp);
      Serial.print(buffer);
      break;
    default: error:
      digitalWrite(kPinPort0, LOW);
      digitalWrite(kPinPort1, LOW);
      while(1) {
        digitalWrite(kPinErrorLed, HIGH); delay(50);
        digitalWrite(kPinErrorLed, LOW); delay(50);
      }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(kPinPort0, OUTPUT);
  pinMode(kPinPort1, OUTPUT);
  pinMode(kPinErrorLed, OUTPUT);
  digitalWrite(kPinPort0, LOW);
  digitalWrite(kPinPort1, LOW);
  digitalWrite(kPinErrorLed, LOW);
}
