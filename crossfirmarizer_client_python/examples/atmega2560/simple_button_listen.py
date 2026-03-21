from crossfirmarizer import ArduinoClient, PinMode
from crossfirmarizer.targets import ATmega2560
import time

if __name__ == "__main__":
    client = ArduinoClient('/dev/tty.usbmodem113301')  # Update with your serial port
    
    # Set pin 13 to INPUT mode
    client.set_gpio(ATmega2560.D8, PinMode.INPUT)
    client.gpio_listen(ATmega2560.D8, True)
    time.sleep(0.1) # Allow time for the listen command to take effect
    try:
        while True:
            pin_state = client.read(ATmega2560.D8)
            print(f"Pin state: {pin_state}")
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()