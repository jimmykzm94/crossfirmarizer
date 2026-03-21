from crossfirmarizer import ArduinoClient, PinMode
from crossfirmarizer.targets import ATmega2560
import time

if __name__ == "__main__":
    client = ArduinoClient('/dev/tty.usbmodem113301')
    
    pin = ATmega2560.D8
    client.set_gpio(pin, PinMode.INPUT)
    
    try:
        while True:
            # ⚠️ There is a latency in reading the button state due to the way the client processes incoming data.
            value = client.digital_read(pin)
            print(f"Pin {pin} value: {value}")
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()