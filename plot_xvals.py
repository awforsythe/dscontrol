import csv
import numpy as np
import matplotlib.pyplot as plt

with open('dump.csv', 'r') as fp:
    reader = csv.reader(fp, delimiter=',')
    headers = next(reader)
    data = np.array(list(reader)).astype(float)

time_vals = data[:, headers.index('time')]
frame_vals = data[:, headers.index('frame')]
x_vals = data[:, headers.index('x')]

prev_frame_val = -1.0
frame_change_times = []
frame_change_ymin = []
frame_change_ymax = []
for i in range(len(frame_vals)):
    frame_val = frame_vals[i]
    if frame_val > prev_frame_val:
        frame_change_times.append(time_vals[i])
        frame_change_ymin.append(x_vals[i] - 0.1)
        frame_change_ymax.append(x_vals[i] + 0.1)
    prev_frame_val = frame_val

plt.plot(time_vals, x_vals)
plt.vlines(frame_change_times, frame_change_ymin, frame_change_ymax, colors='red', label='playtime changes')
plt.xlabel('read time')
plt.ylabel('x position')
plt.tight_layout()
plt.show()
