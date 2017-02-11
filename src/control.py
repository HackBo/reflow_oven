' Communication with the Arduino uno '

from collections import deque
import sys
import time

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

    def __init__(self, oven, curve_file_name, zone_degrees, is_simulation):
        self.oven = oven
        self.curve_file_name = curve_file_name
        self.is_simulation = is_simulation
        # Proportional zone. Only start doing proportional control
        # if the error gets to this zone.
        self.zone = zone_degrees
        self.proporcional_k = 3
        self.integral_k = 1
        self.time_window = 0.2 # seconds
        # Starting time for control/simulation.
        self.time_start = self.current_time()
        self.past_n_error = deque([0.0], maxlen=3)

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

    def sleep(self, seconds):
        ' Sleep for the given mount of seconds '
        if self.is_simulation:
            self.oven.sleep(seconds)
        else:
            time.sleep(seconds)

    def current_time(self):
        ' Get the current time '
        if self.is_simulation:
            return self.oven.current_time()
        else:
            return time.time()

    def aim_for(self, temp_from, time_from, temp_to, time_to):
        ' aim for a given temperature '
        # Compute rect for temperature increase.
        slope = (temp_to - temp_from) / (time_to - time_from)
        const = temp_to - slope * time_to
        # This will take about 0.25s.
        temp_0 = self.oven.read_temp()
        while self.current_time() - self.time_start <= time_to:
            time_in_curve = self.current_time() - self.time_start
            temp_wanted = slope * time_in_curve + const
            print('{} {} {}'.format(round(time_in_curve, 2),
                                    round(temp_0, 2),
                                    round(temp_wanted, 2)))
            error = temp_wanted - temp_0
            self.past_n_error.append(error * self.time_window)

            if error < 0:
                proportion = 0.0
            elif error > self.zone:
                proportion = 1.0
            else:
                integral = self.integral_k * sum(self.past_n_error)
                proportional = (self.proporcional_k * error) / self.zone
                proportion = min(1.0, integral + proportional)
            print('time:{} temp:{} target:{} error:{} proportion:{}'.format(
                round(time_in_curve, 2), round(temp_0, 2), round(temp_wanted, 2),
                round(error, 2), round(proportion, 2)),
                  file=sys.stderr)
            sys.stdout.flush()
            # Only turn on if we need to do some control.
            self.oven.set_output(proportion > 0.0)
            # Time reading from the thermocouple.
            time_thermo = self.current_time()
            temp_0 = self.oven.read_temp()
            time_thermo = self.current_time() - time_thermo
            # How long do we need to be on and off?
            time_need_on = self.time_window * proportion - time_thermo
            time_need_off = self.time_window - time_need_on
            self.sleep(max(0, time_need_on))
            self.oven.set_output(proportion >= 1.0)
            self.sleep(max(0, time_need_off))
