import matplotlib.pyplot as plt
import csv
#import mplcursors

samples = []
voltages = []

with open('ADC_test4_data.csv','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    next(plots)  # Skip the header row
    for row in plots:
        samples.append(float(row[0]))
        voltages.append(float(row[1]))

plt.figure()  # Create a new figure

plt.subplot(2, 1, 1)  # Create the first subplot
plt.plot(samples)
plt.title('Sample Numbers')
plt.ylim([0, 4100])  # Set the limits of the y-axis
plt.ylabel("ADC values (0-4095)")
plt.grid(True)  # Add a grid

plt.subplot(2, 1, 2)  # Create the second subplot
plt.plot(voltages)
plt.title('Voltages')
plt.ylim([0, 3.4])  # Set the limits of the y-axis
plt.ylabel("Voltage (V)")
plt.xlabel("Sample number")
plt.grid(True)  # Add a grid

#mplcursors.cursor(hover=True)  # Enable interactive data cursors

plt.tight_layout()  # Adjust the layout so that the plots do not overlap
plt.show()