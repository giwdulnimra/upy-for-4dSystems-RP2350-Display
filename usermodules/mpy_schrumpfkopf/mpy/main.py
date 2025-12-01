import time
from core import led, console

# 1. LED-Steuerung testen 
led_pin = 0 
led = LED(led_pin)

# 2. Konsolenausgabe testen
console = Console() # Instanziieren

console.info("Starte LED Test...")  #

led.on()
console.info("LED sollte jetzt AN sein.") 
led.status()

time.sleep(5)

led.off()
console.info("LED sollte jetzt AUS sein.")
print(led.status())