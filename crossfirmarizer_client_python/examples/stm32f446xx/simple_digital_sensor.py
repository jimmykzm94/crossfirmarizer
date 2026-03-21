from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import STM32F446RE
import time

if __name__ == "__main__":
    client = SerialClient('/dev/tty.usbmodem113403')
    
    pin = STM32F446RE.PA8
    client.set_gpio(pin, PinMode.INPUT)
    
    try:
        while True:
            value = client.digital_read(pin)
            print(f"Pin {pin} value: {value}")
            time.sleep(0.1)
            # ⚠️ There is a latency in reading the button state due to the way the client processes incoming data.
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()