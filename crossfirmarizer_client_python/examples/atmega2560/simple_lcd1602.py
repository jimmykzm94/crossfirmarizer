from crossfirmarizer import ArduinoClient, LiquidCrystalI2C
import time

if __name__ == "__main__":
    try:
        client = ArduinoClient('/dev/tty.usbmodem113301')
        lcd = LiquidCrystalI2C(client, i2c_address=0x27)
        print("Start LCD demo")
        
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