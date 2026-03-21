from crossfirmarizer import ArduinoClient
from crossfirmarizer.targets import ATmega2560
import time

if __name__ == "__main__":
    client = ArduinoClient('/dev/tty.usbmodem113301')
    client.ultrasonic_config(ATmega2560.D45,ATmega2560.D49,100)
    
    try:
        while True:
            distance = client.ultrasonic_read()
            if distance is not None:
                print(f"Distance: {distance:.2f} cm")
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()
