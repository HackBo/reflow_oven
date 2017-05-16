set title "Reflow!"
set mouse
set key outside below
set autoscale
set term x11
plot "data/oven_program_normal.txt" with linespoints,
plot "oven1.log" using 1:2 lt -1 with lines
pause 1
reread
