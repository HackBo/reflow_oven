' Communication with the Arduino uno '

import logging
import sys
import time

class OvenControl:
    def __init__(oven, curve_file_name, zone_degrees):
        program = read_curve(curve_file_name)
        self.time_start = time.time()
        self.oven = oven
        # Proportional zone. Only start doing proportional control
        # if the error gets to this zone.
        self.zone = zone_degrees
        self.mult = 2
        self.time_window = 3.0 # seconds

    def loop(self):
        ' main control loop '
        time_before = program[0][0]
        temp_before = program[0][1]
        for point in program[1:]:
            aim_for(temp_before, time_before,
            point[1], point[0],
            1.0)
            time_before = point[0]
            temp_before = point[1]
        print >> sys.stderr, "Goal reached. End."

    def read_curve(filename):
        ' Read control curve from file '
        with open(filename) as f_in:
            curve = []
            line = f_in.readline().strip()
            while len(l):
                if line.find('#') == -1:
                    x, y = line.rsplit(' ')
                    curve.append([float(x), float(y)])
             line = f_in.readline().strip()
            return curve

    def aim_for(temp_from, time_from, temp_to, time_to):
        ' aim for a given temperature '
        # Compute rect for temperature increase.
        slope = (temp_to - temp_from) / (time_to - time_from)
        const = temp_to - slope * time_to
        # This will take about 0.25s.
        temp_0 = self.oven.read_temp()
        time_current = time.time() - self.time_start
        while time_current <= time_to:
            # Temperature of oven
            time_curve = time.time() - self.time_start
            temp_wanted = slope * time_curve + const
            print('{} {}'.format(round(time_curve, 2), round(temp_0, 2)))
            sys.stdout.flush()
            error = temp_wanted - temp_0
            if error < 0:
                proportion = 0.0
            else:
                if error > self.zone:
                    proportion = 1.0
                else:
                    proportion = max(1.0, (error * self.mult) / self.zone)
            # Only turn on if we need to do some control.
            self.oven.set_output(proportion > 0.0)
            # Time reading from the thermocouple.
            time_thermo = time.time()
            temp_0 = self.oven.read_temp()
            time_thermo = time.time() - time_thermo
            # How long do we need to be on and off?
            time_need_on = self.time_window * proportion - time_thermo
            time_need_off = self.time_window - time_need_on
            time.sleep(time_need_on)
            self.oven.set_output(False)
            time.sleep(time_need_off)
            time_current = time.time() - self.time_start
