' Communication with the Arduino uno '

import sys
import time
import logging

def read_curve(filename):
    ' Read control curve from file '
    with open(filename) as f_in:
        curve = []
        line = f_in.readline().strip()
        while len(line):
            if line.find('#') == -1:
                spl = [float(x) for x in line.rsplit(' ') if len(x.strip()) > 0]
                curve.append([spl[0], spl[1]])
            line = f_in.readline().strip()
        return curve

class OvenControl:
    ' Control the reflow oven! '

    def __init__(self, oven, curve_file_name, zone_degrees):
        self.time_start = time.time()
        self.oven = oven
        self.curve_file_name = curve_file_name
        # Proportional zone. Only start doing proportional control
        # if the error gets to this zone.
        self.zone = zone_degrees
        self.mult = 2
        self.time_window = 3.0 # seconds

    def follow_curve(self):
        ' main control loop '
        program = read_curve(self.curve_file_name)
        time_before = program[0][0]
        temp_before = program[0][1]
        for point in program[1:]:
            self.aim_for(temp_before, time_before,
                         point[1], point[0])
            time_before = point[0]
            temp_before = point[1]

    def aim_for(self, temp_from, time_from, temp_to, time_to):
        ' aim for a given temperature '
        # Compute rect for temperature increase.
        slope = (temp_to - temp_from) / (time_to - time_from)
        const = temp_to - slope * time_to
        # This will take about 0.25s.
        temp_0 = self.oven.read_temp()
        while time.time() - self.time_start <= time_to:
            time_in_curve = time.time() - self.time_start
            temp_wanted = slope * time_in_curve + const
            print('{} {} {}'.format(round(time_in_curve, 2), round(temp_0, 2), round(temp_wanted, 2)))
            error = temp_wanted - temp_0
            if error < 0:
                proportion = 0.0
            elif error > self.zone:
                proportion = 1.0
            else:
                proportion = (error * self.mult) / self.zone
            logging.info('error: %s', error)
            print('time:{} temp:{} target:{} error:{} proportion:{}'.format(round(time_in_curve, 2), round(temp_0, 2), round(temp_wanted, 2), round(error, 2), round(proportion, 2)), file=sys.stderr)
            sys.stdout.flush()
            #logging.info('error * mult: %s', error * self.mult)
            #logging.info('error * mult / zone: %s', (error * self.mult) / self.zone)
            #logging.info('proportion %s', proportion)
            # Only turn on if we need to do some control.
            self.oven.set_output(proportion > 0.0)
            # Time reading from the thermocouple.
            time_thermo = time.time()
            temp_0 = self.oven.read_temp()
            time_thermo =time.time() - time_thermo
            # How long do we need to be on and off?
            time_need_on = self.time_window * proportion - time_thermo
            time_need_off = self.time_window - time_need_on
            #logging.info('time_on: %s', time_need_on)
            #logging.info('time_off: %s', time_need_off)
            time.sleep(max(0, time_need_on))
            self.oven.set_output(False)
            time.sleep(max(0, time_need_off))
