from crossfirmarizer import SerialClient
from crossfirmarizer.targets import STM32F446RE
import time

if __name__ == "__main__":
    # Update with your serial port
    client = SerialClient('/dev/tty.usbmodem113403')
    client.ultrasonic_config(STM32F446RE.D8,STM32F446RE.D7,100)
    
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
