import pigpio
import time

pulseCount = 0  # Measures flow sensor pulses
VL = 0  # volume in L
flowsensor_pin = 4  # Sensor Input pin 
pump_pin = 22  # 22 for pump

pi = pigpio.pi()  # Create a pigpio object

# Interrupt function
def flow(gpio, level, tick):
    global pulseCount
    pulseCount += 1

# Setup
cb = pi.callback(flowsensor_pin, pigpio.RISING_EDGE, flow)

try:
    pi.write(pump_pin, 1)  # Turn on the pump
    while VL < 0.75: #L 250ml  
        #VL = pulseCount / 4380  # 125 pulses = 1 ml
        VL = pulseCount * 0.0001632514386# + 0.03004
        #print(f"{pulseCount} pulses")
    pi.write(pump_pin, 0)  # Turn off the pump
    print(f"{VL} liters")
    print(f"{pulseCount} pulses")
except KeyboardInterrupt:
    print("\nCleaning up GPIO and exiting...")
    cb.cancel()  # Cancel the callback
    pi.stop()  # Disconnect from the pigpio daemon
    #test 1 for 0.25 we get 255ml
    #test 2 for 0.75 we get 739ml