from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import ATmega2560
import time

PORT = '/dev/tty.usbmodem113402'
PIN = 14

if __name__ == "__main__":
    client = SerialClient(PORT)
    # time.sleep(3)
    
    # Set pin 13 to OUTPUT mode
    client.set_gpio(PIN, PinMode.OUTPUT)
    
    # Blink the LED on pin 13
    try:
        while True:
            print("Turning LED ON")
            client.digital_write(PIN, 0)  # Turn LED on, active low
            time.sleep(0.5)
            print("Turning LED OFF")
            client.digital_write(PIN, 1)  # Turn LED off
            time.sleep(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        print("Cleaning up: Turning LED OFF and closing client.")
        client.digital_write(PIN, 1)  # Turn LED off
        client.close()