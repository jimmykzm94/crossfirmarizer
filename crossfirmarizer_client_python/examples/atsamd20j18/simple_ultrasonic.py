from crossfirmarizer import SerialClient
from crossfirmarizer.targets import STM32F446RE
import time

PORT = '/dev/tty.usbmodem113402'
TRIG = 2+32
ECHO = 4+32

if __name__ == "__main__":
    # Update with your serial port
    client = SerialClient(PORT)
    client.ultrasonic_config(TRIG,ECHO,100)
    
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
