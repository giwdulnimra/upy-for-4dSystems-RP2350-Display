from machine import Pin, UART
import time

led = Pin(45, Pin.OUT)
uart = UART(1, baudrate=115200, tx=Pin(4), rx=Pin(5))

def toggle_led():
    led.toggle()

while True:
    if uart.any():  # Check if there is any data in the UART buffer
        received_data = uart.read(1)  # Read one byte of data
        if received_data:
            data_char = received_data.decode('utf-8')  # Decode the byte to string
            print(f"Received data: {data_char}")  # Print the received data
            if data_char == '1':  # If '1' is received, turn the LED on
                led.on()
            elif data_char == '0':  # If '0' is received, turn the LED off
                led.off()
            elif data_char == 'T':  # If 'T' is received, toggle the LED
                toggle_led()

    time.sleep(0.1)  # Small delay to avoid high CPU usage

