from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import STM32F446RE
import time

PORT = '/dev/tty.usbmodem113402'
PIN = 15

if __name__ == "__main__":
    client = SerialClient(PORT)
    
    client.set_gpio(PIN, PinMode.INPUT_PULLUP)
    
    try:
        while True:
            value = client.digital_read(PIN)
            print(f"Button value: {value}")
            time.sleep(0.1)
            # ⚠️ There is a latency in reading the button state due to the way the client processes incoming data.
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()