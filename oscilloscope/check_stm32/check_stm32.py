import matplotlib.pyplot as plt
import csv
import numpy as np
#import mplcursors

samples = []
voltages = []

with open('stm32data.csv','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    next(plots)  # Skip the header row
    for row in plots:
        samples.append(float(row[0]))
        voltages.append(float(row[1]))

# Perform the FFT
fft_result = np.fft.fft(voltages)
# Compute the frequency axis (assuming a sample rate of 470 kHz)
sample_rate = 500000
freq = np.fft.fftfreq(len(voltages), 1/sample_rate)

# Compute the absolute values of the FFT results
fft_abs = np.abs(fft_result)
max_index = np.argmax(fft_abs)

# Find the frequency corresponding to this index
max_freq = freq[max_index]
print("max freq (Hz): ", max_freq)

# Create a figure
plt.figure()

# Plot the signal
plt.subplot(2, 1, 1)
plt.plot(voltages, label='Data')
plt.ylim([-2, 2])  # Set the limits of the y-axis
plt.title('Signal')
plt.xlabel('Time')
plt.ylabel('Amplitude')
plt.legend()  # Add a legend
plt.grid(True)  # Add a grid

# Plot the frequency response
plt.subplot(2, 1, 2)
plt.plot(np.abs(freq), np.abs(fft_result))
plt.title('Frequency Response')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.grid(True)  # Add a grid

#plt.tight_layout()
plt.show()