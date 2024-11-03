from gpiozero import LED, Button
from threading import Timer

led = LED(22)  # 22 for pump
button = Button(17)  # 17 for pulse counting

pulse_count = 0  # Initialize pulse count

def turn_off_led():
    led.off()  # Turn off the LED
    print(f"Total pulses: {pulse_count}")  # Print total pulse count

def count_pulse():
    global pulse_count
    pulse_count += 1  # Increment pulse count

# Set up pulse counting
button.when_pressed = count_pulse

# Turn on the LED
led.on()

# Start a timer for 10 seconds, after which turn_off_led will be called
timer = Timer(10, turn_off_led)
timer.start()

# Wait for the timer thread to finish
timer.join()