import numpy as np
import matplotlib.pyplot as plt

# Load the data from the CSV file
#data = np.loadtxt('Triggered/trig49.csv', skiprows=1, usecols=0)
# Get the number from the user
num = input("Enter the file nb: ")

# Add the number to the end of the filename
filename = 'Triggered/trig' + num + '.csv'
data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
datafilt = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=1)
freq_values = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=2)
magnitude = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=3)
magnitudenotfilt = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=4)
thresh = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=5)





# Create a figure
plt.figure()

# Plot the signal
plt.subplot(2, 1, 1)
# plt.plot(data, label='Data_not_filtered')
# plt.plot(datafilt, label='Data_filtered',color='red')
# plt.plot(thresh, label='Threshold', linestyle='--', color='green')
# plt.ylim([-2050, 2050])  # Set the limits of the y-axis
# plt.title('Signal')
# plt.xlabel('Time')
# plt.ylabel('Amplitude')
# plt.legend()  # Add a legend
# plt.grid(True)  # Add a grid


##for report
# Set your sampling frequency
fs = 500e3  # 500kHz

# Create a time array in milliseconds
t = np.arange(len(data)) / fs * 1000  # Multiply by 1000 to convert seconds to milliseconds

# Plot the data against time
plt.plot(t, data)
plt.title('Time Series Data')
plt.xlabel('Time (ms)')  # Change x-axis label to 'Time (ms)'
plt.ylabel('Amplitude (12-bit quantisation)')  # Change y-axis label to 'Amplitude (12-bit quantisation
plt.grid(True)

# Set x-axis limits to match the range of the time series
plt.xlim(t[0], t[-1])

# Plot the frequency response
plt.subplot(2, 1, 2)
plt.plot(freq_values[:1023], magnitudenotfilt[:1023], label='Magnitude not filtered')
plt.plot(freq_values[:1023], magnitude[:1023], label='Magnitude filtered', color='red')
plt.title('Frequency Response')
plt.xlabel('Frequency')
plt.ylabel('Magnitude')
plt.legend()  # Add a legend
plt.grid(True)  # Add a grid


# Adjust the layout and display the plots
plt.tight_layout()
plt.show()