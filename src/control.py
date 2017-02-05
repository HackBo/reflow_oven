' Communication with the Arduino uno '

import logging
import sys
import time

import oven

def aim_for (t0, oven, yprev, tprev, ynext, tnext, deltat):
    slope = (ynext - yprev) / (tnext - tprev)
    const = ynext - slope * tnext
    tnow = time.time() - t0
    while tnow <= tnext:
      time.sleep(deltat)
      ynow0 = oven.read_temp()
      tnow = time.time() - t0
      guess = slope * tnow  + const

      oven.set_output(ynow0 < guess)

      #log = '{} {} {} {}'.format(round(tnow, 2), round(tnext, 2), round(ynow0, 2), round(ynext, 2))
      log = '{} {}'.format(round(tnow, 2), round(ynow0, 2))
      print(log)
      sys.stdout.flush()
      print('time temp', file=sys.stderr)
      print(log, file=sys.stderr)
      sys.stderr.flush()

def main():
    ' Our main funcion. Open Arduino and send pings '
    logging.info('Be safe!')
    logging.info('Trying to open port %s', sys.argv[1])
    arduino = arduinocmd.Arduino(port=sys.argv[1], baudrate=9600)
    time.sleep(1.0)
    # Sending ping.
    oven = oven.OvenOne(arduino)
    oven.select(0)
    program = read_curve('splinedata')
    time_before = program[0][0]
    temp_before = program[0][1]
    t0 = time.time()
    for v in program[1:]:
      aim_for(t0, oven, temp_before, time_before, v[1], v[0], 0.25)
      time_before = v[0]
      temp_before = v[1]

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
