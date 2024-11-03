import signal
import os
import time
import threading
import sys
import pigpio

# Set up the GPIO pin
pump_pin = 22 #pump
led_pin = 23 #led
flow_pin = 4 #flow sensor

pi = pigpio.pi()

lock = threading.Lock()
c_pid = None

# Initialize the counter
counter = 0
threshPerPeriod = 3 #threshold of nb of plant sound count per period
checkPeriod = 20 #seconds, check nb of plant sound count

pulseCount = 0  # Measures flow sensor pulses
VL = 0  # volume in L
VLerror = -0.017519 # L error from best fit line

# Interrupt function
def flow(gpio, level, tick):
    global pulseCount
    pulseCount += 1

# Setup
pi.set_mode(flow_pin, pigpio.INPUT)
pi.set_pull_up_down(flow_pin, pigpio.PUD_DOWN)
cb = pi.callback(flow_pin, pigpio.RISING_EDGE, flow)

def handler(signum, frame):
    if signum == signal.SIGUSR1:
        global counter
        counter += 1
    else: 
        print('Signal handler called with signal ', signum, ' exiting')
        cb.cancel()
        pi.stop()
        sys.exit(0) 

def check_counter():
    global counter
    global checkPeriod
    global threshPerPeriod
    global c_pid
    global pulseCount
    global VL
    global time_thread

    print('\n////////////frequency of signals:', counter, 'signals per', checkPeriod, ' seconds////////////')
    
    if counter > threshPerPeriod:
        os.kill(c_pid, signal.SIGUSR1)#stop listening to sound
        print('plants need water\n\n\n')
        pi.write(led_pin, 1)
        VL = 0
        pulseCount = 0
        pi.write(pump_pin, 1)
        while VL < 0.568: #L 250ml
            VL = pulseCount * 0.0001632514386 
            #VL = pulseCount * 0.000167172027
        pi.write(pump_pin, 0)
        print(f"{round(VL,3)} liters") 
        #print('pulseCount: ', pulseCount) 
        os.kill(c_pid, signal.SIGUSR1)#start listening to sound again
    else:
        print('plants are fine\n\n\n')
        pi.write(led_pin, 0)
    
    counter = 0 # Reset the counter
    
    time_thread = threading.Timer(checkPeriod, check_counter)
    time_thread.daemon = True
    time_thread.start()

# Set the signal handler
signal.signal(signal.SIGUSR1, handler)
signal.signal(signal.SIGTERM, handler) #signal for kill from bash

time.sleep(3) #wait for c code to start

# Wait for the target PID to be set
while c_pid is None:
    with open('c_pid.txt', 'r') as f:
        c_pid = int(f.read())
    os.remove('c_pid.txt')
    if c_pid is None:
        time.sleep(1)  # Wait for 1 second

# Start the timer
time_thread = threading.Timer(checkPeriod, check_counter)
time_thread.daemon = True
time_thread.start()

while True:
    pass