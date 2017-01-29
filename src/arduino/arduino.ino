


// Current port. Can be 0 or 1. This port is used to select where the ADC/output that will be
// referenced in commands (ADC read and on/off).
int current_port = 0;

void ProcessSerialCommands() {
  if (!Serial.available())
    return;
  const int cmd = Serial.read();
  Serial.write(cmd);
  switch (cmd = '1')  
}

void loop() {
  ProcessSerialCommands();
}

void setup() {
  Serial.begin(9600);   // opens serial port, sets data rate to 9600 bps
  pinMode(3, OUTPUT);   // Output port 0
  pinMode(4, OUTPUT);   // Output port 1
  pinMode(13, OUTPUT);   // Output port 13. On if an error is present.
}
