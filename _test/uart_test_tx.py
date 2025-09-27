from machine import Pin, UART
import machine
import rp2
import time

set_led = False
led = Pin("LED", Pin.OUT)
# Initialize UART
uart = UART(0, baudrate=115200, tx=0, rx=1)

def cb_uart_led(uart_led_timer):
    global set_led
    set_led = not set_led
    uart_led(set_led)

def uart_led(ch):
    led.value(ch)
    if ch:
        uart.write('1')
    else:
        uart.write('0')

uart_led_timer = machine.Timer()
uart_led_timer.init(period=1000, mode=machine.Timer.PERIODIC, callback=cb_uart_led)

while True:
    if rp2.bootsel_button():
        uart_led_timer.deinit()
        break