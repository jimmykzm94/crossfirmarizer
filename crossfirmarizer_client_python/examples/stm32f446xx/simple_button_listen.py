from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import STM32F446RE
import time

if __name__ == "__main__":
    client = SerialClient('/dev/tty.usbmodem113403')

    client.set_gpio(STM32F446RE.BUTTON, PinMode.INPUT)
    client.gpio_listen(STM32F446RE.BUTTON, True)
    time.sleep(0.1) # Allow time for the listen command to take effect
    try:
        while True:
            pin_state = client.read(STM32F446RE.BUTTON)
            print(f"Pin state: {pin_state}")
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()