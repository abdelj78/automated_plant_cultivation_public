import matplotlib.pyplot as plt
import numpy as np

plt.ion()  # Turn on interactive mode

for i in range(100):
    x = np.linspace(0, 4*np.pi, 1000)
    y = np.sin(x + i/10.0)
    plt.clf()  # Clear the plot
    plt.plot(x, y)
    plt.pause(0.1)  # Pause for a short period

plt.ioff()  # Turn off interactive mode