from gpiozero import Button
import time

flow_frequency = 0  # Measures flow sensor pulses
l_hour = 0  # Calculated litres/hour
flowsensor = Button(17)  # Sensor Input pin 
cloopTime = time.time()

# Interrupt function
def flow():
    global flow_frequency
    flow_frequency += 1

# Setup
flowsensor.when_pressed = flow

try:
    while True:
        currentTime = time.time()
        # Every second, calculate and print litres/hour
        if currentTime >= (cloopTime + 1):
            cloopTime = currentTime  # Updates cloopTime
            # Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
            l_hour = (flow_frequency * 60 / 7.5)  # (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
            flow_frequency = 0  # Reset Counter
            print(f"{l_hour} L/hour")  # Print litres/hour
except KeyboardInterrupt:
    print("\nCleaning up GPIO and exiting...")








#####################RPi.GPIO is outdated for this kernel version as event_detect doesn't work, use gpiozero instead#################
# import RPi.GPIO as GPIO
# import time

# GPIO.cleanup()

# flow_frequency = 0  # Measures flow sensor pulses
# l_hour = 0  # Calculated litres/hour
# flowsensor = 27  # Sensor Input pin 
# cloopTime = time.time()

# # Interrupt function
# def flow(channel): #channel is used to tell the function which pin called it but here only flow pin used
#     global flow_frequency
#     flow_frequency += 1

# # Setup
# GPIO.setmode(GPIO.BCM)
# GPIO.setup(flowsensor, GPIO.IN, pull_up_down=GPIO.PUD_UP)
# GPIO.add_event_detect(flowsensor, GPIO.RISING, callback=flow)

# try:
#     while True:
#         currentTime = time.time()
#         # Every second, calculate and print litres/hour
#         if currentTime >= (cloopTime + 1):
#             cloopTime = currentTime  # Updates cloopTime
#             # Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
#             l_hour = (flow_frequency * 60 / 7.5)  # (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
#             flow_frequency = 0  # Reset Counter
#             print(f"{l_hour} L/hour")  # Print litres/hour
# except KeyboardInterrupt:
#     print("\nCleaning up GPIO and exiting...")
#     GPIO.cleanup()