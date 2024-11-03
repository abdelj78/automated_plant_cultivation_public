import joblib
from joblib import load
import sklearn
import pandas as pd
import librosa
import signal
import os
import time
import threading

import sys
import numpy as np
from scipy import stats
print('PYTHON/ Starting the Python script')

gpio_pin = 23

filename = 'relay.csv'
c_pid = None
# print(joblib.__version__)
# print(sklearn.__version__)
# print(pd.__version__)
print("version: ",librosa.__version__)
print("file: ",librosa.__file__)

# Initialize the counter
counter = 0
checkPeriod = 20

svc_poly = load('svc_polyv2.joblib')
print('PYTHON/ Model loaded')