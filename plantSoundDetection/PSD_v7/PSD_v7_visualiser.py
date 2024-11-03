import numpy as np
import matplotlib.pyplot as plt

# Load the data from the CSV file
#data = np.loadtxt('Triggered/trig49.csv', skiprows=1, usecols=0)
# Get the number from the user
num = input("Enter the file nb: ")

# Add the number to the end of the filename
#filename = 'Triggered_plant/trig' + num + '.csv' #DONT FOGET TO CHANGE PATH FOR PLANT AND NOT PLANT
filename = 'C:/Users/abdel/Documents/GitHub/year4_project/plantSoundDetection/PSD_v7/Triggered_plant_exp1_results/trig' + num + '.csv'
#filename = 'Triggered_no_plant/trig' + num + '.csv' 
#data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
#freq_values = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=1)
#magnitude = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=2)
#magnitudenotfilt = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=4)
thresh = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=1)

fft_result = np.fft.fft(data)
# Compute the frequency axis (assuming a sample rate of 470 kHz)
sample_rate = 500000
freq = np.fft.fftfreq(len(data), 1/sample_rate)



# Create a figure
plt.figure()

# Plot the signal
plt.subplot(2, 1, 1)
plt.plot(data, label='Data_not_filtered')
#plt.plot(datafilt, label='Data_filtered',color='red')
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
plt.plot(np.abs(freq), np.abs(fft_result), label='Magnitude not filt', color='blue')
plt.title('Frequency Response')
plt.xlabel('Frequency')
plt.ylabel('Magnitude')
plt.legend()  # Add a legend
plt.grid(True)  # Add a grid


# # Adjust the layout and display the plots
# plt.tight_layout()
plt.show()