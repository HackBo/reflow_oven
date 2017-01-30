const int kPinPort0 = 3;
const int kPinPort1 = 4;
const int kPinAdc0 = A0;
const int kPinAdc1 = A1;
const int kPinLed = 13;

// Current port. Can be 0 or 1. This port is used to select where the ADC/output that will be
// referenced in commands (ADC read and on/off).
int current_port = 0;

void ProcessSerialCommands() {
  if (!Serial.available())
    return;
  const int cmd = Serial.read();
  Serial.write(cmd);
  switch (cmd) {
    int adc;
    case '0':
      current_port = 0;
      break;
    case '1':
      current_port = 1;
      break;
    case '+': case '-':
      digitalWrite(current_port ? kPinPort1 : kPinPort0, cmd == '+' ? HIGH : LOW);
      // FIXME(nelson): Delete the next line.
      //digitalWrite(kPinLed, cmd == '+' ? HIGH : LOW);
      break;
    case 'A':
      adc = analogRead(current_port ? kPinAdc1 : kPinAdc0);
      Serial.write((adc >> 8) & 0xff);
      Serial.write(adc & 0xff);
      break;
    default:
      digitalWrite(kPinPort0, LOW);
      digitalWrite(kPinPort1, LOW);
      while(1) {
        digitalWrite(kPinLed, HIGH); delay(50);
        digitalWrite(kPinLed, LOW); delay(50);
      }
  }
}

void loop() {
  ProcessSerialCommands();
}

void setup() {
  Serial.begin(9600);   // opens serial port, sets data rate to 9600 bps
  pinMode(kPinPort0, OUTPUT);   // Output port 0
  pinMode(kPinPort1, OUTPUT);   // Output port 1
  pinMode(kPinLed, OUTPUT);   // Output port 13. On if an error is present.
}
