'''
Supported 1602 for now
'''
from .crossfirmarizer import SerialClient
from time import sleep

# commands
LCD_CLEARDISPLAY = 0x01
LCD_RETURNHOME = 0x02
LCD_ENTRYMODESET = 0x04
LCD_DISPLAYCONTROL = 0x08
LCD_CURSORSHIFT = 0x10
LCD_FUNCTIONSET = 0x20
LCD_SETCGRAMADDR = 0x40
LCD_SETDDRAMADDR = 0x80

# flags for display entry mode
LCD_ENTRYRIGHT = 0x00
LCD_ENTRYLEFT = 0x02
LCD_ENTRYSHIFTINCREMENT = 0x01
LCD_ENTRYSHIFTDECREMENT = 0x00

# flags for display on/off control
LCD_DISPLAYON = 0x04
LCD_DISPLAYOFF = 0x00
LCD_CURSORON = 0x02
LCD_CURSOROFF = 0x00
LCD_BLINKON = 0x01
LCD_BLINKOFF = 0x00

# flags for display/cursor shift
LCD_DISPLAYMOVE = 0x08
LCD_CURSORMOVE = 0x00
LCD_MOVERIGHT = 0x04
LCD_MOVELEFT = 0x00

# flags for function set
LCD_8BITMODE = 0x10
LCD_4BITMODE = 0x00
LCD_2LINE = 0x08
LCD_1LINE = 0x00
LCD_5x10DOTS = 0x04
LCD_5x8DOTS = 0x00

# flags for backlight control
LCD_BACKLIGHT = 0x08
LCD_NOBACKLIGHT = 0x00

En = 0b00000100  # Enable bit
Rw = 0b00000010  # Read/Write bit
Rs = 0b00000001  # Register select bit

class LiquidCrystalI2C:
    def __init__(self, client: SerialClient, i2c_address=0x27, sda_pin=0, scl_pin=0, i2c_bus=0):
        self.client = client
        self.i2c_address = i2c_address
        self.sda_pin = sda_pin
        self.scl_pin = scl_pin
        self.i2c_bus = i2c_bus
        self.backlight = True
        self.client.i2c_config(self.i2c_bus, self.sda_pin, self.scl_pin)
        self.begin()

    def begin(self):
        sleep(0.05)  # wait for LCD to power up
        self.expander_write(self.backlight)
        sleep(0.05)

        # Initialize LCD in 4-bit mode
        self.write4bits(0x03 << 4)
        sleep(0.005)
        self.write4bits(0x03 << 4)
        sleep(0.005)
        self.write4bits(0x03 << 4)
        sleep(0.001)
        self.write4bits(0x02 << 4)

        # Set function, display control, and entry mode
        self.send_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS)
        self.send_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF)
        self.send_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT)

    def expander_write(self, data):
        if self.backlight:
            data |= LCD_BACKLIGHT
        else:
            data &= ~LCD_BACKLIGHT
        self.client.i2c_write(self.i2c_bus, self.i2c_address, bytes([data]))

    def write4bits(self, data):
        self.expander_write(data)
        self.pulse_enable(data)

    def pulse_enable(self, data):
        self.expander_write(data | En)
        sleep(0.000001)
        self.expander_write(data & ~En)
        sleep(0.00005)

    def send(self, value, mode):
        high_nibble = value & 0xF0
        low_nibble = (value << 4) & 0xF0
        self.write4bits(high_nibble | mode)
        self.write4bits(low_nibble | mode)

    def send_command(self, value):
        self.send(value, 0)

    def send_data(self, value):
        self.send(value, Rs)

    def clear(self):
        self.send_command(LCD_CLEARDISPLAY)
        sleep(0.002)

    def home(self):
        self.send_command(LCD_RETURNHOME)
        sleep(0.002)

    def set_cursor(self, col, row):
        row_offsets = [0x00, 0x40, 0x14, 0x54]
        if row > 1:
            row = 1
        self.send_command(LCD_SETDDRAMADDR | (col + row_offsets[row]))

    def write(self, text):
        for char in text:
            self.send_data(ord(char))

    def backlight_on(self):
        self.backlight = True
        self.expander_write(0)

    def backlight_off(self):
        self.backlight = False
        self.expander_write(0)

    def display_on(self):
        self.send_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON)

    def display_off(self):
        self.send_command(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF)

    def cursor_on(self):
        self.send_command(LCD_DISPLAYCONTROL | LCD_CURSORON)

    def cursor_off(self):
        self.send_command(LCD_DISPLAYCONTROL | LCD_CURSOROFF)

    def blink_on(self):
        self.send_command(LCD_DISPLAYCONTROL | LCD_BLINKON)

    def blink_off(self):
        self.send_command(LCD_DISPLAYCONTROL | LCD_BLINKOFF)

    def scroll_display_left(self):
        self.send_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT)

    def scroll_display_right(self):
        self.send_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT)

    def left_to_right(self):
        self.send_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT)

    def right_to_left(self):
        self.send_command(LCD_ENTRYMODESET | LCD_ENTRYRIGHT)

    def autoscroll_on(self):
        self.send_command(LCD_ENTRYMODESET | LCD_ENTRYSHIFTINCREMENT)

    def autoscroll_off(self):
        self.send_command(LCD_ENTRYMODESET | LCD_ENTRYSHIFTDECREMENT)

    def create_char(self, location, charmap):
        location &= 0x7
        self.send_command(LCD_SETCGRAMADDR | (location << 3))
        for i in range(8):
            self.send_data(charmap[i])

# application example
def print_hello_world(lcd):
    # lcd.power_reset()
    # sleep(1)
    lcd.clear()
    # sleep(1)
    lcd.set_cursor(0, 0)
    # sleep(1)
    lcd.write("Hello, World!")
    # sleep(1)


def word_from_left_to_right(lcd, word):
    lcd.clear()
    for i in range(len(word)):
        lcd.set_cursor(i, 0)
        lcd.write(word[i])
        sleep(0.1)

def word_scroll_from_right_to_left(lcd, word):
    # Assuming a 16x2 LCD, we can only display 16 characters at a time
    lcd.clear()
    display_width = 16
    padded_word = " " * display_width + word + " " * display_width
    for i in range(len(padded_word) - display_width + 1):
        lcd.set_cursor(0, 0)
        lcd.write(padded_word[i:i+display_width])
        sleep(0.1)
    
def jumping_word(lcd, word, line, cycles=1, interval=0.1):
    if len(word) > 16:
        raise ValueError("Word must be 16 characters or less")
    if line not in [0, 1]:
        raise ValueError("Line must be 0 or 1")
    
    # one cycle means the word jumps from left to right and back to left
    lcd.clear()
    for _ in range(cycles):
        # jump from left to right
        for i in range(0, 16 - len(word)):
            lcd.set_cursor(i, line)
            lcd.write(word)
            sleep(interval)
            lcd.set_cursor(i, line)
            lcd.write(" " * len(word))  # clear the word before next position

        # jump from right to left
        for i in range(16 - len(word), -1, -1):
            lcd.set_cursor(i, line)
            lcd.write(word)
            sleep(interval)
            lcd.set_cursor(i, line)
            lcd.write(" " * len(word))  # clear the word before next position

def word_blink(lcd, word, col_row=[0,0], cycles=1, interval=0.5):
    if len(word) > 16:
        raise ValueError("Word must be 16 characters or less")
    if col_row[0] < 0 or col_row[0] > 15 or col_row[1] < 0 or col_row[1] > 1:
        raise ValueError("Cursor position must be within the LCD bounds")
    
    lcd.clear()
    for _ in range(cycles):
        # display the word
        lcd.set_cursor(col_row[0], col_row[1])
        lcd.write(word)
        sleep(interval)

        # clear the word
        lcd.set_cursor(col_row[0], col_row[1])
        lcd.write(" " * len(word))
        sleep(interval)

