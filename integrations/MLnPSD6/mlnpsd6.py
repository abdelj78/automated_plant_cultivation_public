import joblib
from joblib import load
import sklearn
import pandas as pd
import librosa
import signal
import os
import time
import threading
import RPi.GPIO as GPIO
import sys
import numpy as np
from scipy import stats


print('PYTHON/ Starting the Python script')
print(librosa.__version__)
gpio_pin = 23
# Set up the GPIO pin
GPIO.setmode(GPIO.BCM)  # Use Broadcom pin numbering
GPIO.setup(gpio_pin, GPIO.OUT)  # Set pin 18 as an output pin
filename = 'relay.csv'
c_pid = None
# print(joblib.__version__)
# print(sklearn.__version__)
# print(pd.__version__)
# print(librosa.__version__)

# Initialize the counter
counter = 0
checkPeriod = 20

#svc_poly = load('svc_polyv2.joblib')
pipeline = load('svc_polyv4.pkl')
print('PYTHON/ Model loaded')
# Initialize an empty DataFrame to store the results
df = pd.DataFrame(columns=['Energy', 'Energy Entropy', 'Spectral Entropy', 'Max Frequency'] + [f'MFCC Mean {i+1}' for i in range(13)])


def handler(signum, frame):
    if signum == signal.SIGUSR1:
        global counter
        global filename
        global c_pid
        global timer
        print('PYTHON/ Signal handler called with signal ', signum, ' will process sound now')
        #print('Counter:', counter)
        features = feature_extraction(filename)
        # Collect all the features into a list
        input_features = [features[0], features[1], features[2], features[3]] + list(features[4])
        # Create a Series with the input features and the column names of df
        row = pd.Series(input_features, index=df.columns)
        # Write the series to the second row of df
        df.loc[1] = row

        #df_std = pipeline.transform(df)
        #print("\ninput features: ", df_std)
        # Use the model to predict the class of the input
        predicted_class = pipeline.predict(df) #apparently need to pass a list of lists because predict expects a 2D array

        print('\nPredicted class:', predicted_class)
        if predicted_class == 'plant':
            counter += 1
            print('PYTHON/ Plant sound detected')
        else:
            print('PYTHON/ Not a plant sound')

        #check of audio file done let c code access csv file
        os.kill(c_pid, signal.SIGUSR1)


    else: 
        print('PYTHON/ Signal handler called with signal ', signum, ' exiting')
        # Cancel the timer
        if timer is not None:
            timer.cancel()
        
        GPIO.cleanup()
        sys.exit(0) 

def check_counter():
    global counter
    global checkPeriod
    global timer
    print('\nPYTHON/ ////////////////frequency of signals:', counter, 'signals per 20 seconds//////////////////////')
    if counter > 3:
        print('PYTHON/ plants need water\n\n\n')
        # Turn on the GPIO pin
        GPIO.output(gpio_pin, GPIO.HIGH)
        # Wait for 1 second
        time.sleep(5)
        # Turn off the GPIO pin
        GPIO.output(gpio_pin, GPIO.LOW)
    else:
        print('PYTHON/ plants are fine\n\n\n')
        # GPIO.cleanup()
        # sys.exit(0) 
        
    # Reset the counter
    counter = 0
    # Start another timer
    timer = threading.Timer(checkPeriod, check_counter).start()


def feature_extraction(filename):
    # Read the wav file
    data = np.genfromtxt(filename, delimiter=',', skip_header=1, usecols=0)
    sample_rate = 500000
    #normalise data
    datamax = np.max(np.abs(data))
    data = data/datamax
    # total energy
    energy = np.sum(np.square(data))
    # energy entropy
    energy_entropy = stats.entropy(np.abs(data))
    # spectral entropy
    fft_result = np.fft.rfft(data)
    fft_freq = np.fft.rfftfreq(len(data), 1/sample_rate)
    spectral_entropy = stats.entropy(np.abs(fft_result))
    # max frequency
    max_peak_index = np.argmax(np.abs(fft_result))
    max_freq = fft_freq[max_peak_index]
    # MFCCs
    mfccs = librosa.feature.mfcc(y=data, sr=sample_rate, n_mfcc=13, n_fft=512, hop_length=256)
    #mfccs = librosa.feature.mfcc(y=data, sr=sample_rate, n_mfcc=13, n_fft=512, hop_length=256, n_mels=40)
    # Mean of MFCCs
    mfccs_mean = np.mean(mfccs, axis=1)

    return energy, energy_entropy, spectral_entropy, max_freq, list(mfccs_mean)



# Set the signal handler
signal.signal(signal.SIGUSR1, handler)
#signal.signal(signal.SIGINT, handler) #
signal.signal(signal.SIGTERM, handler) #signal for kill from bash

# Start the timer
threading.Timer(checkPeriod, check_counter).start()

# #try: 
# while True:
#     pass
# # except KeyboardInterrupt:
# #     # Clean up the GPIO pins before exiting
# #     GPIO.cleanup()


# Wait for a short period of time to ensure the bash script has written the PID
time.sleep(3)
while True:
    # Wait for the target PID to be set
    while c_pid is None:
        # Replace this with the actual code to get the target PID
        # Then read the PID from the file
        with open('c_pid.txt', 'r') as f:
            c_pid = int(f.read())
        # Delete the file
        os.remove('c_pid.txt')
        # If the target PID is still None, wait for a while before checking again
        if c_pid is None:
            time.sleep(1)  # Wait for 1 second
            
    pass
