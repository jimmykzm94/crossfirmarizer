from crossfirmarizer import ArduinoClient, PinMode
from crossfirmarizer.targets import ATmega2560
import time

if __name__ == "__main__":
    client = ArduinoClient('/dev/tty.usbmodem113301')
    
    # Set pin 13 to INPUT mode
    client.set_gpio(ATmega2560.D8, PinMode.INPUT)
    
    try:
        while True:
            value = client.digital_read(ATmega2560.D8)
            print(f"Button value: {value}")
            time.sleep(0.1)
            # ⚠️ There is a latency in reading the button state due to the way the client processes incoming data.
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()