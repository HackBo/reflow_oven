' Main function for oven control. Real oven and simulated one '

import argparse
import logging
import sys
import time

import arduinocmd
from control import OvenControl
from oven import RealOvenOne, FakeOvenOne

def main():
    ' Our main funcion. '

    # Parse args.
    parser = argparse.ArgumentParser()
    parser.add_argument("--simulate", help="Use the oven simulator")
    parser.add_argument("-p", "--port", type=str,
                        help="Serial port to open (not used in simulation mode)")
    args = parser.parse_args()

    # Get the proper oven.
    if args.simulate:
        logging.info('Simulated mode')
        oven = FakeOvenOne()
    else:
        logging.info('Trying to open port %s', args.port)
        arduino = arduinocmd.Arduino(port=args.port, baudrate=9600)
        time.sleep(2.0)
        oven = RealOvenOne(arduino) # pylint: disable=R0204
    oven.select(0)
    control = OvenControl(oven=oven,
                          curve_file_name='data/oven_program_normal.txt',
                          zone_degrees=15,
                          is_simulation=args.simulate)
    control.follow_curve()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
