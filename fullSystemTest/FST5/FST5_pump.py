import signal
import os
import time
import threading
import RPi.GPIO as GPIO
import sys

# Set up the GPIO pin
pump_pin = 22 #pump
led_pin = 23 #led
GPIO.setmode(GPIO.BCM)  # Use Broadcom pin numbering
GPIO.setup(led_pin, GPIO.OUT)  # Set pin 18 as an output pin
GPIO.setup(pump_pin, GPIO.OUT)  # Set pin 18 as an output pin
GPIO.output(led_pin, GPIO.LOW)
GPIO.output(pump_pin, GPIO.LOW)


# Initialize the counter
counter = 0
threshPerPeriod = 3 #threshold of nb of plant sound count per period
checkPeriod = 20 #seconds, check nb of plant sound count


def handler(signum, frame):
    if signum == signal.SIGUSR1:
        global counter
        counter += 1
        #print('PYTHON/ Signal handler called with signal ', signum, ' plant sound counter: ', counter)
    else: 
        print('PYTHON/ Signal handler called with signal ', signum, ' exiting')
        GPIO.cleanup()
        sys.exit(0) 
        

def check_counter():
    global counter
    global checkPeriod
    global threshPerPeriod
    print('\nPYTHON/ ////////////////frequency of signals:', counter, 'signals per 20 seconds//////////////////////')
    
    if counter > threshPerPeriod:
        print('PYTHON/ plants need water\n\n\n')
        GPIO.output(led_pin, GPIO.HIGH) 
        GPIO.output(pump_pin, GPIO.HIGH)      
    else:
        print('PYTHON/ plants are fine\n\n\n')
        GPIO.output(led_pin, GPIO.LOW)
        GPIO.output(pump_pin, GPIO.LOW)
    
    counter = 0 # Reset the counter
    
    # Start another timer
    threading.Timer(checkPeriod, check_counter).start()

# Set the signal handler
signal.signal(signal.SIGUSR1, handler)
#signal.signal(signal.SIGINT, handler) #
signal.signal(signal.SIGTERM, handler) #signal for kill from bash

# Start the timer
threading.Timer(checkPeriod, check_counter).start()

#try: 
while True:
    pass
# except KeyboardInterrupt:
#     # Clean up the GPIO pins before exiting
#     GPIO.cleanup()