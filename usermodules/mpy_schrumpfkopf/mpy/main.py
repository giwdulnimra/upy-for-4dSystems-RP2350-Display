import time
import core

LED_PIN = 0
my_console = core.console() # the C-Wrapper registers the classes in lowercase for mpy
my_led = core.led(LED_PIN)  # even thou the C++-classes are called 'Console' and 'LedDriver'


my_console.info("Starting LED Test on Pin %d...", LED_PIN)

my_led.on()
my_console.info("LED should be ON now.")

status_bool = my_led.status()
print("Python verification: LED is", "ON" if status_bool else "OFF")

time.sleep(5)

my_led.off()
my_console.info("LED should be OFF now.")

status_bool = my_led.status()
print("Python verification: LED is", "ON" if status_bool else "OFF")

my_console.info("Test finished.")