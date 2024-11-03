import signal
import os
import time
import threading
import RPi.GPIO as GPIO
import sys
gpio_pin = 23
# Set up the GPIO pin
GPIO.setmode(GPIO.BCM)  # Use Broadcom pin numbering
GPIO.setup(gpio_pin, GPIO.OUT)  # Set pin 18 as an output pin

# # Turn on the GPIO pin
# GPIO.output(gpio_pin, GPIO.HIGH)
# # Wait for 1 second
# time.sleep(1)
# # Turn off the GPIO pin
# GPIO.output(gpio_pin, GPIO.LOW)


# Initialize the counter
counter = 0
checkPeriod = 20

def handler(signum, frame):
    if signum == signal.SIGUSR1:
        global counter
        counter += 1
        print('PYTHON/ Signal handler called with signal ', signum, ' plant sound counter: ', counter)
        #print('Counter:', counter)
    else: 
        print('PYTHON/ Signal handler called with signal ', signum, ' exiting')
        GPIO.cleanup()
        sys.exit(0) 
        

def check_counter():
    global counter
    global checkPeriod
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