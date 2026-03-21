from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import STM32F446RE
import time

if __name__ == "__main__":
    client = SerialClient('/dev/tty.usbmodem113403')
    
    client.set_gpio(STM32F446RE.BUTTON, PinMode.INPUT)
    
    try:
        while True:
            value = client.digital_read(STM32F446RE.BUTTON)
            print(f"Button value: {value}")
            time.sleep(0.1)
            # ⚠️ There is a latency in reading the button state due to the way the client processes incoming data.
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()