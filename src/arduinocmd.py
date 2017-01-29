' Communication with the Arduino uno '

import logging
import sys
import serial

class Arduino:
    ' Communication with the arduino via serial port '

    def __init__(self, port, baudrate):
        self.port = serial.Serial(port, baudrate, timeout=1) # 8,N,1

    def sendbyte(self, byte):
        ' Send a byte'
        self.port.write(byte)

    def readbyte(self):
        ' Read a byte '
        char = self.port.read()
        if len(char) == 0:
            return False, ''
        return True, char

    def readline(self):
        ' Try to read a line from the serial port. \n is the ending char. '
        ret = ''
        while 1:
            char = self.port.read()
            if len(char) == 0:
                # Timeout
                return False, ''
            ret += char.decode('ascii')
            if char == b'\n':
                return True, ret.strip()
def main():
    ' Our main funcion. Open Arduino and send pings '
    arduino = Arduino(port=sys.argv[1], baudrate=9600)
    logging.info('Opened port %s', sys.argv[1])
    arduino.sendbyte(b'0')
    print(arduino.readbyte())
    arduino.sendbyte(b'1')
    print(arduino.readbyte())

if __name__ == "__main__":
    main()
