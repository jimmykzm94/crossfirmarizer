from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import STM32F446RE
import time

if __name__ == "__main__":
    # Update with your serial port
    client = SerialClient('/dev/tty.usbmodem113403')
    
    client.set_gpio(STM32F446RE.LED, PinMode.OUTPUT)
    
    # Blink the LED on pin 13
    try:
        while True:
            print("Turning LED ON")
            client.digital_write(STM32F446RE.LED, 1)  # Turn LED on
            time.sleep(0.5)
            print("Turning LED OFF")
            client.digital_write(STM32F446RE.LED, 0)  # Turn LED off
            time.sleep(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        print("Cleaning up: Turning LED OFF and closing client.")
        client.digital_write(STM32F446RE.LED, 0)  # Turn LED off
        client.close()