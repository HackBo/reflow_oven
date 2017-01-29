' Communication with the Arduino uno '

import arduinocmd
import logging
import sys
import time

def main():
    ' Our main funcion. Open Arduino and send pings '
    logging.info('Be safe!')
    logging.info('Trying to open port %s', sys.argv[1])
    arduino = arduinocmd.Arduino(port=sys.argv[1], baudrate=9600)
    logging.info('Opened port %s and sending pings.', sys.argv[1])
    # Sending ping.
    arduino.sendbyte(b'1')
    arduino.sendbyte(b'0')
    if arduino.readbyte() != (True, b'1') or arduino.readbyte() != (True, b'0'):
        logging.error('Ping to port %s failed. Bailing out. Be safe.', sys.argv[1])
    logging.info('Ping OK!')

    #time.sleep(2)
    #arduino.sendbyte(b'-')
    #print(arduino.readbyte())

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    main()
