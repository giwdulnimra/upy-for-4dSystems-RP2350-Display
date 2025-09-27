from machine import Pin
from time import sleep
from picozero import pico_led

led = Pin(45, Pin.OUT)
# led = Pin('ADC5', Pin.OUT)
# led = Pin('SPI_CS', Pin.OUT)

for i in range(40):
    sleep(0.25)
    led.high()
    pico_led.off()
    sleep(0.25)
    led.low()
    pico_led.on()
pico_led.off()

