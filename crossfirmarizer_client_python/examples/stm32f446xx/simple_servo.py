from crossfirmarizer import SerialClient, ServoSG90
from crossfirmarizer.targets import STM32F446RE
import time

if __name__ == "__main__":
    client = SerialClient('/dev/tty.usbmodem113403')
    servo = ServoSG90(client, STM32F446RE.CN7_PIN_0)
    
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