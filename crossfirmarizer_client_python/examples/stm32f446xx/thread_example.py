from crossfirmarizer import SerialClient, PinMode
from crossfirmarizer.targets import STM32F446RE
import time
import threading
import queue

th_running = True

def display_message(display_queue):
    global th_running
    states = {}
    while th_running:
        try:
            key, value = display_queue.get(timeout=0.1)
            states[key] = value
            
            # Build output string from states
            output = " | ".join(f"{k}: {v:<3}" for k, v in states.items())
            # \r returns cursor to the start, and \033[K clears out any leftover characters to the right
            print(f"\r\033[K{output}", end="", flush=True)
        except queue.Empty:
            pass

def led_blink(client, display_queue):
    global th_running
    client.set_gpio(STM32F446RE.LED, PinMode.OUTPUT)
    while th_running:
        display_queue.put(("LED", "ON"))
        client.digital_write(STM32F446RE.LED, 1)  # Turn LED on
        time.sleep(0.3)
        display_queue.put(("LED", "OFF"))
        client.digital_write(STM32F446RE.LED, 0)  # Turn LED off
        time.sleep(0.3)

def button_example(client, display_queue):
    global th_running
    client.set_gpio(STM32F446RE.BUTTON, PinMode.INPUT)
    while th_running:
        value = client.digital_read(STM32F446RE.BUTTON)
        display_queue.put(("Button", value))
        time.sleep(0.1)

def ultrasonic_example(client, display_queue):
    global th_running
    client.ultrasonic_config(STM32F446RE.D8,STM32F446RE.D7,200)
    while th_running:
        distance = client.ultrasonic_read()
        if distance is not None:
            display_queue.put(("Distance", f"{distance:.2f} cm"))
        time.sleep(0.1)

if __name__ == "__main__":
    # Update with your serial port
    client = SerialClient('/dev/tty.usbmodem113403')

    # create queue
    display_queue = queue.Queue()

    # create led, button, and ultrasonic thread
    th_display = threading.Thread(target=display_message, args=(display_queue,))
    th_led = threading.Thread(target=led_blink, args=(client, display_queue))
    th_button = threading.Thread(target=button_example, args=(client, display_queue))
    th_ultrasonic = threading.Thread(target=ultrasonic_example, args=(client, display_queue))

    th_display.start()
    th_led.start()
    th_button.start()
    th_ultrasonic.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print() # Move to a new line when exiting, so "Closing client" prints cleanly
        pass
    finally:
        print("Closing client.")
        th_running = False
        th_display.join()
        th_led.join()
        th_button.join()
        th_ultrasonic.join()
        client.close()