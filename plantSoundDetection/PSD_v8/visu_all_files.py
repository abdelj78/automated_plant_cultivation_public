import glob
import numpy as np
import matplotlib.pyplot as plt
import heapq

# Get a list of all CSV files in the directory
filenames = glob.glob('C:/Users/abdel/Documents/GitHub/year4_project/plantSoundDetection/PSD_v7/Test1Th64mV/*.csv')

# Create a list to store the peak magnitudes and filenames
peaks = []

# Loop over all files
for filename in filenames:
    # Load the data from the file
    data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)

    # Compute the FFT and the frequency axis
    fft_result = np.fft.fft(data)
    sample_rate = 500000
    freq = np.fft.fftfreq(len(data), 1/sample_rate)

    # Find the peak frequency and magnitude
    peak_freq = freq[np.argmax(np.abs(fft_result))]
    peak_magnitude = np.max(np.abs(fft_result))

    # Check if the peak frequency is between 36 kHz and 60 kHz
    if 36000 <= peak_freq <= 60000:
        # Add the peak magnitude and filename to the list
        peaks.append((peak_magnitude, filename))

# Get the 10 files with the highest peak magnitudes
top_peaks = heapq.nlargest(10, peaks)

# Loop over the top 10 files
for peak_magnitude, filename in top_peaks:
    # Load the data from the file
    data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
    thresh = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=1)
    # Plot the signal
    plt.figure()

    # Plot the signal
    plt.subplot(2, 1, 1)
    plt.plot(data, label='Data_not_filtered')
    #plt.plot(datafilt, label='Data_filtered',color='red')
    plt.plot(thresh, label='Threshold', linestyle='--', color='green')
    plt.ylim([-2050, 2050])  # Set the limits of the y-axis
    plt.title(f'Signal {filename} with Peak Frequency: {peak_freq/1000} kHz')
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

    # plt.plot(freq/1000, np.abs(fft_result))
    # plt.xlabel('Frequency (kHz)')
    # plt.ylabel('Magnitude')
    # plt.title(f'FFT of {filename} with Peak Frequency: {peak_freq/1000} kHz')
    plt.show()