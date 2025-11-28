import core
import time

# 1. LED-Steuerung testen (verwenden Sie einen Pin, der auf Ihrem Board verfügbar ist)
# Wir verwenden GPIO 0 als Beispielpin
led_pin = 0 
led = widget_core.Led(led_pin)

# 2. Konsolenausgabe testen
console = widget_core.Console()

console.debug("Starte LED Test...") 
# Sollte auf der Debug-Konsole erscheinen (printf-Ausgang)

led.turn_on()
console.info("LED sollte jetzt AN sein.") 
# Sollte auf der REPL erscheinen
print(led)
time.sleep(1)

led.turn_off()
console.info("LED sollte jetzt AUS sein.")
print(led)