import numpy as np
import matplotlib.pyplot as plt

# Load the data from the CSV file
#data = np.loadtxt('Triggered/trig49.csv', skiprows=1, usecols=0)
# Get the number from the user
num = input("Enter the file nb: ")

# Add the number to the end of the filename
filename = 'Triggered/trig' + num + '.csv'
#data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
datafilt = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
freq_values = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=1)
magnitude = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=2)
#magnitudenotfilt = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=4)
thresh = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=3)





# Create a figure
plt.figure()

# Plot the signal
plt.subplot(2, 1, 1)
#plt.plot(data, label='Data_not_filtered')
plt.plot(datafilt, label='Data_filtered',color='red')
plt.plot(thresh, label='Threshold', linestyle='--', color='green')
plt.ylim([-2050, 2050])  # Set the limits of the y-axis
plt.title('Signal')
plt.xlabel('Time')
plt.ylabel('Amplitude')
plt.legend()  # Add a legend
plt.grid(True)  # Add a grid

# Plot the frequency response
plt.subplot(2, 1, 2)
#plt.plot(freq_values[:1023], magnitudenotfilt[:1023], label='Magnitude not filtered')
plt.plot(freq_values[:511], magnitude[:511], label='Magnitude filtered', color='red')
plt.title('Frequency Response')
plt.xlabel('Frequency')
plt.ylabel('Magnitude')
plt.legend()  # Add a legend
plt.grid(True)  # Add a grid


# Adjust the layout and display the plots
plt.tight_layout()
plt.show()