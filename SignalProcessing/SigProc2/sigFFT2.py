import numpy as np
import matplotlib.pyplot as plt

# Load the data from the CSV file
data = np.loadtxt('CleanData2.csv', delimiter=',', skiprows=1, usecols=0)
datafiltered = np.loadtxt('CleanData2.csv', delimiter=',', skiprows=1, usecols=1)
# Perform the FFT
fft_result = np.fft.fft(data)
fft_resultfiltered = np.fft.fft(datafiltered)
# Compute the frequency axis (assuming a sample rate of 470 kHz)
sample_rate = 470000
freq = np.fft.fftfreq(len(data), 1/sample_rate)

# Create a figure
plt.figure()

# Plot the signal
plt.subplot(2, 2, 1)
plt.plot(data)
plt.title('Signal')
plt.xlabel('Time')
plt.ylabel('Amplitude')

# Plot the frequency response
plt.subplot(2, 2, 3)
plt.plot(np.abs(freq), np.abs(fft_result))
plt.title('Frequency Response')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')

# Plot the frequency response
plt.subplot(2, 2, 2)
plt.plot(datafiltered)
plt.title('Signal filtered')
plt.xlabel('Time')
plt.ylabel('Amplitude')

# Plot the frequency response
plt.subplot(2, 2, 4)
plt.plot(np.abs(freq), np.abs(fft_resultfiltered))
plt.title('FFT of filtered signal')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')

# Adjust the layout and display the plots
plt.tight_layout()
plt.show()