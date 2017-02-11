' Communication with the Arduino uno '

import sys

class RealOvenOne:
    ' Oven : Send commands to oven over serial port '
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
        ''' Read owen temperature for selected thermocouple
            You need a delay of about 1/4s between reads. '''
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

class FakeOvenOne:
    ' Oven: simulated oven for testing '

    def __init__(self):
        self.simulated_time = 0.0
        self.temp = 22.0
        self.is_on = False
        # How much to increase per second when turned on.
        self.deg_sec_up = 0.8
        # How much to decrease per second when turned off.
        self.deg_sec_down = 0.05

    def select(self, who):
        ' Select thermocouple to read. Select output to write.'
        assert who == 0 or who == 1
        print('Selection not supported in simulated oven',
              file=sys.stderr)

    def set_output(self, value):
        ' Set output to true of on or off value '
        assert isinstance(value, bool)
        self.is_on = value

    def read_temp(self):
        ' Read simulated temperature '
        return self.temp

    def sleep(self, seconds):
        ' simulate sleep and change temperature '
        if self.is_on:
            self.temp += seconds * self.deg_sec_up
            self.temp = min(240.0, self.temp)
        else:
            self.temp -= seconds * self.deg_sec_down
            self.temp = max(22.0, self.temp)
        self.simulated_time += seconds

    def current_time(self):
        ' Get the simulated time '
        self.simulated_time += 0.0001 # Never return the same temp.
        return self.simulated_time
