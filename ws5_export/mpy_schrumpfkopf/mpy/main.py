import cpp_hardware
import time

# 1. LED-Objekt erstellen
led = cpp_hardware.LED()

# 2. Status ausgeben (ruft den custom Print-Callback auf)
print(led) # Ausgabe: <LED-Objekt LED (Pin 25): AUS>

# 3. LED ansteuern
print("LED AN...")
led.on()
time.sleep(1)

# 4. Status prüfen und erneut ausgeben
print("LED Status:", led.is_on()) # Ausgabe: LED Status: True
print(led) # Ausgabe: <LED-Objekt LED (Pin 25): AN>

# 5. Ausschalten
print("LED AUS...")
led.off()
time.sleep(1)