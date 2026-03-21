from crossfirmarizer import ArduinoClient, ServoSG90
from crossfirmarizer.targets import ATmega2560
import time

if __name__ == "__main__":
    client = ArduinoClient('/dev/tty.usbmodem113301')
    servo = ServoSG90(client, ATmega2560.D6)
    
    try:
        angles = [0, 90, 180, 90]  # Example angles to test
        while True:
            for angle in angles:
                print(f"Setting angle to {angle} degrees")
                servo.write_angle(angle)
                time.sleep(1)  # Wait a bit between changes
    except KeyboardInterrupt:
        pass
    finally:
        print("Closing client.")
        client.close()