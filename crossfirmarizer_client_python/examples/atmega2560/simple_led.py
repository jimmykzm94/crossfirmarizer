from crossfirmarizer import ArduinoClient, PinMode
from crossfirmarizer.targets import ATmega2560
import time

if __name__ == "__main__":
    client = ArduinoClient('/dev/tty.usbmodem113301')
    
    # Set pin 13 to OUTPUT mode
    client.set_gpio(ATmega2560.LED, PinMode.OUTPUT)
    
    # Blink the LED on pin 13
    try:
        while True:
            print("Turning LED ON")
            client.digital_write(ATmega2560.LED, 1)  # Turn LED on
            time.sleep(0.5)
            print("Turning LED OFF")
            client.digital_write(ATmega2560.LED, 0)  # Turn LED off
            time.sleep(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        print("Cleaning up: Turning LED OFF and closing client.")
        client.digital_write(ATmega2560.LED, 0)  # Turn LED off
        client.close()