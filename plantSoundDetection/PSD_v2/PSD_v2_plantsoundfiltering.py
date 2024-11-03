import numpy as np
import matplotlib.pyplot as plt

# Load the data from the CSV file
import os
import pandas as pd
import shutil

directory = 'Triggered'
filtered_directory = 'UltrasoundDetected'

for filename in os.listdir(directory):
    if filename.endswith(".csv"):
        df = pd.read_csv(os.path.join(directory, filename))
        data = df.iloc[:, 0].values
        # Perform operations on the dataframe df here
        # Perform the FFT
        fft_result = np.fft.fft(data)
        # Compute the frequency axis (assuming a sample rate of 470 kHz)
        sample_rate = 470000
        freq = np.fft.fftfreq(len(data), 1/sample_rate)
        # Compute the absolute values of the FFT results
        fft_abs = np.abs(fft_result)

        # Find the index of the maximum value in the FFT results
        max_index = np.argmax(fft_abs)

        # Find the frequency corresponding to this index
        max_freq = freq[max_index]
        if 20000 < max_freq < 100000:
            shutil.copy(os.path.join(directory, filename), filtered_directory)
            
            #or can use this
            # new_file_path = os.path.join(new_directory, filename)
            # df.to_csv(new_file_path, index=False)
