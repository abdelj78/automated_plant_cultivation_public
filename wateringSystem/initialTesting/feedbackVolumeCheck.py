from gpiozero import Button, LED
import time

pulseCount = 0  # Measures flow sensor pulses
VL = 0  # volume in L
VLerror = -0.017519 # L error from best fit line
flowsensor = Button(4)  # Sensor Input pin 
pump = LED(22)  # 22 for pump

# Interrupt function
def flow():
    global pulseCount
    pulseCount += 1

# Setup
flowsensor.when_pressed = flow

try:
    pump.on()
    while VL < 0.25: #L 250ml  
        #VL = pulseCount / 4380  # 125 pulses = 1 ml
        VL = pulseCount * 0.0001883688 
    pump.off()
    print(f"{VL} liters")
    print(f"{pulseCount} pulses")
except KeyboardInterrupt:
    flowsensor.close()
    pump.close()
    print("\nCleaning up GPIO and exiting...")



