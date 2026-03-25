from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import ATmega2560
import time

PORT = '/dev/tty.usbmodem113402'
PIN = 15

if __name__ == "__main__":
    client = SerialClient(PORT)  # Update with your serial port
    
    # Set pin 13 to INPUT mode
    client.set_gpio(PIN, PinMode.INPUT_PULLUP)
    client.gpio_listen(PIN, True)
    time.sleep(0.1) # Allow time for the listen command to take effect
    try:
        while True:
            pin_state = client.read(PIN)
            print(f"Pin state: {pin_state}")
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()