from machine import Pin
import utime

# Define which pin number controls the LED
led_pin_number: int = 16

pulse_pin_number: int = 4 #red
dir_pin_number: int = 5 #yellow
driver_ena_pin_number:int = 8 #white
shifter_ena_pin_number: int = 9 #green

# Initialize pico pin
pulse_pin: Pin = Pin(pulse_pin_number, Pin.OUT)
dir_pin: Pin = Pin(dir_pin_number, Pin.OUT)
driver_ena_pin: Pin = Pin(driver_ena_pin_number, Pin.OUT)
shifter_ena_pin: Pin = Pin(shifter_ena_pin_number, Pin.OUT)

print("Enabling Level Shifter...")
shifter_ena_pin.value(1)
utime.sleep_ms(100)
print("Level shifter should now be enabled...")

print("Enabling Driver...")
driver_ena_pin.value(1)
utime.sleep_ms(100)
print("Driver should now be enabled...")

print("Setting direction..")
dir_pin.value(1)
utime.sleep_ms(100)
print("Direction set..")


# Blink pin
while True:        
    pulse_pin.value(1)        
    utime.sleep_us(8)        
    pulse_pin.value(0)
    utime.sleep_us(8)    
