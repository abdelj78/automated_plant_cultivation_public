import numpy as np
import matplotlib.pyplot as plt

# Load the data from the CSV file
#data = np.loadtxt('Triggered/trig49.csv', skiprows=1, usecols=0)
filename = 'Triggered/trig1.csv'
data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
thresh = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=1)

# Perform the FFT
fft_result = np.fft.fft(data)
# Compute the frequency axis (assuming a sample rate of 470 kHz)
sample_rate = 500000
freq = np.fft.fftfreq(len(data), 1/sample_rate)

# Compute the absolute values of the FFT results
fft_abs = np.abs(fft_result)
max_index = np.argmax(fft_abs)

# Find the frequency corresponding to this index
max_freq = freq[max_index]
print("max freq (Hz): ", max_freq)

# Create a time array
t = np.arange(len(data)) / sample_rate

# Create a figure
plt.figure()

# Plot the signal
plt.subplot(2, 1, 1)
plt.plot(t, data, label='Data')
plt.plot(thresh, label='Threshold', linestyle='--', color='red')
plt.ylim([-2050, 2050])  # Set the limits of the y-axis
# Set x-axis limits to match the range of the time series
plt.xlim(t[0], t[-1])
plt.title('Signal')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude (12-bit quantisation)')
plt.legend()  # Add a legend
plt.grid(True)  # Add a grid

# Plot the frequency response
plt.subplot(2, 1, 2)
plt.plot(np.abs(freq), np.abs(fft_result))
plt.title('Frequency Response')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.grid(True)  # Add a grid

# # Plot the frequency response
# plt.subplot(2, 2, 2)
# plt.plot(datafiltered)
# plt.title('Signal filtered')
# plt.xlabel('Time')
# plt.ylabel('Amplitude')

# # Plot the frequency response
# plt.subplot(2, 2, 4)
# plt.plot(np.abs(freq), np.abs(fft_resultfiltered))
# plt.title('FFT of filtered signal')
# plt.xlabel('Frequency (Hz)')
# plt.ylabel('Magnitude')

# Adjust the layout and display the plots
plt.tight_layout()
plt.show()