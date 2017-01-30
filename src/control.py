' Communication with the Arduino uno '

import logging
import sys
import time

import arduinocmd


class OvenOne:
    ' Controller for the oven '
    def __init__(self, arduino):
        self.arduino = arduino

    def get_adc(self):
        ' Get an adc value from bytes '
        status, high = self.arduino.readbyte()
        if not status:
            return False, -1
        status, low = self.arduino.readbyte()
        if not status:
            return False, -1
        return True, ord(high) << 8 | ord(low)

    def adc_to_volts(self, adc):
        ' Convert adc value to voltage '
        v_max = 5.0
        v_min = 0.0
        adc_max = 1023
        adc_min = 0
        slope = (v_max - v_min) / (adc_max - adc_min)
        constant = v_max -  slope * adc_max
        return slope * adc + constant

    def loop(self):
        ' Print ADC0 in a loop '
        while True:
            self.arduino.sendbyte(b'A') # Read ADC
            assert self.arduino.readbyte() == (True, b'A')
            status, adc = self.get_adc()
            assert status
            print(adc, self.adc_to_volts(adc))
            time.sleep(1)

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

    # FIXME(remove)
    #arduino.sendbyte(b'+')
    #assert arduino.readbyte() == (True, b'+')

    oven = OvenOne(arduino)
    oven.loop()

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    main()
