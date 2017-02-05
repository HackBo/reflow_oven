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

data = read_curve('oven_program.txt')
for v in data:
    print(v[0] / 1.7, v[1])
