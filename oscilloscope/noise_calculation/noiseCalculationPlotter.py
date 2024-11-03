import matplotlib.pyplot as plt
import csv
#import mplcursors

values = []
voltages = []
values_filtered = []
voltages_filtered = []

with open('noise_calculation_data.csv','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    next(plots)  # Skip the header row
    for row in plots:
        values.append(float(row[0]))
        voltages.append(float(row[1]))
        values_filtered.append(float(row[2]))
        voltages_filtered.append(float(row[3]))

plt.figure()  # Create a new figure

plt.subplot(2, 2, 1)  # Create the first subplot
plt.plot(values)
plt.title('Sample Numbers')
plt.ylim([0, 4100])  # Set the limits of the y-axis
plt.ylabel("ADC values (0-4095)")
plt.grid(True)  # Add a grid

plt.subplot(2, 2, 3)  # Create the second subplot
plt.plot(voltages)
plt.title('Voltages')
plt.ylim([0, 3.4])  # Set the limits of the y-axis
plt.ylabel("Voltage (V)")
plt.xlabel("Sample number")
plt.grid(True)  # Add a grid

plt.subplot(2, 2, 2)  # Create the second subplot
plt.plot(values_filtered)
plt.title('values_filtered')
plt.ylim([-2050, 2050])  # Set the limits of the y-axis
plt.ylabel("ADC values (0-4095)")
plt.xlabel("Sample number")
plt.grid(True)  # Add a grid

plt.subplot(2, 2, 4)  # Create the second subplot
plt.plot(voltages_filtered)
plt.title('Voltages_filtered')
plt.ylim([-1.65, 1.65])  # Set the limits of the y-axis
plt.ylabel("Voltage (V)")
plt.xlabel("Sample number")
plt.grid(True)  # Add a grid

#mplcursors.cursor(hover=True)  # Enable interactive data cursors

plt.tight_layout()  # Adjust the layout so that the plots do not overlap
plt.show()