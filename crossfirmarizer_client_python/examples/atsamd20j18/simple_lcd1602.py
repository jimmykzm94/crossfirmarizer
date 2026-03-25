from crossfirmarizer import SerialClient, LiquidCrystalI2C
import time

PORT = '/dev/tty.usbmodem113402'
SDA = 8
SCL = 9
BUS = 2

if __name__ == "__main__":
    try:
        client = SerialClient(PORT)
        lcd = LiquidCrystalI2C(client, i2c_address=0x27, sda_pin=SDA, scl_pin=SCL, i2c_bus=BUS)
        print("Start LCD demo")
        lcd.clear()
        lcd.set_cursor(0,0)
        lcd.write("hello me")
        time.sleep(5)

        # lcd.clear()
        
        # TODO add these function into the class
        # word_from_left_to_right(lcd, "Hello, World!")
        # word_scroll_from_right_to_left(lcd, "Hello, World! This is a scrolling text demo.")
        # jumping_word(lcd, "Bello!", 0, cycles=1, interval=0.2)
        # print_hello_world(lcd)

        # middle of the screen
        # word = "Blink!"
        # middle_col = (16 - len(word)) // 2
        # word_blink(lcd, word, [middle_col, 0], cycles=5, interval=0.5)
    finally:
        client.close()