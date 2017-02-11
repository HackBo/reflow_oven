' Communication with the Arduino uno '

import logging
import sys
import time

import arduinocmd
from control import OvenControl
from oven import RealOvenOne

def main():
    ' Our main funcion. Open Arduino and send pings '
    logging.info('Trying to open port %s', sys.argv[1])
    arduino = arduinocmd.Arduino(port=sys.argv[1], baudrate=9600)
    time.sleep(2.0)
    oven = RealOvenOne(arduino)
    oven.select(0)
    control = OvenControl(oven, 'data/oven_program_normal.txt', zone_degrees=15)
    control.follow_curve()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
