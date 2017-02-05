' Communication with the Arduino uno '

import logging
import sys
import time

import arduinocmd

class OvenOne:
    ' Controller for the oven '
    def __init__(self, arduino):
        self.arduino = arduino

    def send_cmd(self, cmd):
        ' Send a command '
        cmd_out = bytes(cmd.encode('ascii'))
        self.arduino.sendbyte(cmd_out)
        status, cmd_in = self.arduino.readbyte()
        if not status or cmd_in != cmd_out:
            print('Error sending command!', file=sys.stderr)
            sys.exit(1)

    def select(self, who):
        ' Select thermocouple to read. Select output to write.'
        assert who == 0 or who == 1
        self.send_cmd(chr(ord('0') + who))

    def set_output(self, value):
        ' Set output to true of on or off value '
        assert isinstance(value, bool)
        self.send_cmd('+' if value else '-')

    def read_temp(self):
        ' Read owen temperature for selected thermocouple '
        self.send_cmd('T')
        num_str = ''
        for _ in range(7):
            status, in_byte = self.arduino.readbyte()
            if not status:
                break
            if in_byte == b'.':
                return float(num_str) / 4.0
            num_str += in_byte.decode('ascii')
        print('Error receiving temp!', file=sys.stderr)
        sys.exit(1)

    def loop2(self, temp_target):
        ' Print ADC0 in a loop '
        while True:
            for who in [0, 1]:
                self.select(who)
                self.set_output(True)
                self.read_temp()
                #time.sleep(0.2)
                self.set_output(False)
            time.sleep(0.5)

    def loop(self, temp_target):
        ' Print ADC0 in a loop '
        self.select(0)
        while True:
            temp = self.read_temp()
            print(temp)
            self.set_output(temp < 40)
            time.sleep(1.0)

def main():
    ' Our main funcion. Open Arduino and send pings '
    logging.info('Be safe!')
    logging.info('Trying to open port %s', sys.argv[1])
    arduino = arduinocmd.Arduino(port=sys.argv[1], baudrate=9600)
    logging.info('Opened port %s. Lets wait and send pings.', sys.argv[1])
    time.sleep(1.0)
    # Sending ping.
    arduino.sendbyte(b'1')
    arduino.sendbyte(b'0')
    if arduino.readbyte() != (True, b'1') or arduino.readbyte() != (True, b'0'):
        logging.error('Ping to port %s failed. Bailing out. Be safe.', sys.argv[1])
    logging.info('Ping OK!')
    # Start owen loop. With a fixed temperature, just for a test.
    oven = OvenOne(arduino)
    oven.loop(temp_target=103)

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    main()
