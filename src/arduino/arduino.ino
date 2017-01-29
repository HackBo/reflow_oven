const int kPinPort0 = 3;
const int kPinPort1 = 4;
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
    case '0':
      current_port = 0;
      break;
    case '1':
      current_port = 1;
      break;
    case '+': case '-':
      digitalWrite(current_port ? kPinPort1 : kPinPort0, cmd == '+' ? HIGH : LOW);
      break;
    default:
      digitalWrite(kPinLed, HIGH);
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
