import signal
import os
import time
import threading
#import RPi.GPIO as GPIO
import sys
from gpiozero import Button, LED

# Set up the GPIO pin
# pump_pin = 22 #pump
# led_pin = 23 #led
# GPIO.setmode(GPIO.BCM)  # Use Broadcom pin numbering
# GPIO.setup(led_pin, GPIO.OUT)  # Set pin 18 as an output pin
# GPIO.setup(pump_pin, GPIO.OUT)  # Set pin 18 as an output pin
# GPIO.output(led_pin, GPIO.LOW)
# GPIO.output(pump_pin, GPIO.LOW)


lock = threading.Lock()
c_pid = None

# Initialize the counter
counter = 0
threshPerPeriod = 3 #threshold of nb of plant sound count per period
checkPeriod = 20 #seconds, check nb of plant sound count


pulseCount = 0  # Measures flow sensor pulses
VL = 0  # volume in L
#VLerror = -0.0154 # L original error from 250ml only
VLerror = -0.017519 # L error from best fit line
flowsensor = Button(4)  # Sensor Input pin 
pump = LED(22)  # 22 for pump
blueled = LED(23)  # 23 for blue led

# Interrupt function
def flow():
    global pulseCount
    #with lock:
    pulseCount += 1

# Setup
flowsensor.when_pressed = flow

def handler(signum, frame):
    if signum == signal.SIGUSR1:
        global counter
        counter += 1
        #print('PYTHON/ Signal handler called with signal ', signum, ' plant sound counter: ', counter)
    else: 
        print('PYTHON/ Signal handler called with signal ', signum, ' exiting')
        flowsensor.close()
        pump.close()
        blueled.close()
        sys.exit(0) 
        

def check_counter():
    global counter
    global checkPeriod
    global threshPerPeriod
    global c_pid
    global pulseCount
    global VL
    global pump
    global blueled
    global time_thread

    print('\nPYTHON/ ////////////////frequency of signals:', counter, 'signals per', checkPeriod, ' seconds//////////////////////')
    
    if counter > threshPerPeriod:
        os.kill(c_pid, signal.SIGUSR1)#stop listening to sound
        print('PYTHON/ plants need water\n\n\n')
        blueled.on()
        VL = 0
        pulseCount = 0
        pump.on()
        while VL < 0.25: #L 250ml
            #with lock:  #lock doesn't seem to make a difference 
            #VL = pulseCount / 4380  # 125 pulses = 1 ml
            VL = pulseCount * 0.0001883688 + VLerror
            #print(f"{pulseCount} pulses")
        pump.off() 
        print(f"{round(VL,3)} liters") 
        print('pulseCount: ', pulseCount) 
        os.kill(c_pid, signal.SIGUSR1)#start listening to sound again
    else:
        print('PYTHON/ plants are fine\n\n\n')
        blueled.off()
    
    counter = 0 # Reset the counter
    
    # Start another timer
    #threading.Timer(checkPeriod, check_counter).start()

    time_thread = threading.Timer(checkPeriod, check_counter)
    time_thread.daemon = True
    time_thread.start()

# Set the signal handler
signal.signal(signal.SIGUSR1, handler)
#signal.signal(signal.SIGINT, handler) #
signal.signal(signal.SIGTERM, handler) #signal for kill from bash




time.sleep(3) #wait for c code to start
#try: 


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

# Start the timer
time_thread = threading.Timer(checkPeriod, check_counter)
time_thread.daemon = True
time_thread.start()

while True:
    pass

# while True: 
#     start = time.time()
#     while time.time() - start < checkPeriod:
#         pass 

#     print('\nPYTHON/ ////////////////frequency of signals:', counter, 'signals per ',checkPeriod,' seconds//////////////////////')
    
#     if counter > threshPerPeriod:
#         os.kill(c_pid, signal.SIGUSR1)#stop listening to sound
#         print('PYTHON/ plants need water\n\n\n')
#         blueled.on()
#         VL = 0
#         pulseCount = 0
#         pump.on()
#         while VL < 0.25: #L 250ml
#             #with lock:  
#             VL = pulseCount / 4380  # 125 pulses = 1 ml
#             # print(f"{pulseCount} pulses")
#             print(f"{VL} liters")
#         pump.off()   
#         os.kill(c_pid, signal.SIGUSR1)#start listening to sound again
#     else:
#         print('PYTHON/ plants are fine\n\n\n')
#         blueled.off()
    
#     counter = 0 # Reset the counter
    
    