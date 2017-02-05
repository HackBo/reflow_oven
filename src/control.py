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
        ' Do not read too fast or it will not work '
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

def aim_for (t0, oven, yprev, tprev, ynext, tnext, deltat):
    slope = (ynext - yprev) / (tnext - tprev)
    const = ynext - slope * tnext
    tnow = time.time() - t0
    while tnow <= tnext:
      time.sleep(deltat)
      ynow0 = oven.read_temp()
      tnow = time.time() - t0
      #guess = slope * (tnow + deltat * (30 / deltat))  + const
      guess = slope * (tnow + 30 )  + const

      oven.set_output(ynow0 < guess)

      #log = '{} {} {} {}'.format(round(tnow, 2), round(tnext, 2), round(ynow0, 2), round(ynext, 2))
      log = '{} {}'.format(round(tnow, 2), round(ynow0, 2))
      print(log)
      sys.stdout.flush()
      print('time temp', file=sys.stderr)
      print(log, file=sys.stderr)
      sys.stderr.flush()

def read_curve(filename):
    f = open(filename, 'r')
    v = []
    l = f.readline().strip()
    while len(l):
      if l.find('#') == -1:
        x, y = l.rsplit(' ')
        v.append([float(x), float(y)])
      l = f.readline().strip()
    f.close()
    return v

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
    oven.select(0)
    program = read_curve('splinedata')
    time_before = program[0][0]
    temp_before = program[0][1]
    t0 = time.time()
    for v in program[1:]:
      aim_for(t0, oven, temp_before, time_before, v[1], v[0], 0.25)
      time_before = v[0]
      temp_before = v[1]
    print >> sys.stderr, "Goal reached. End."

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
