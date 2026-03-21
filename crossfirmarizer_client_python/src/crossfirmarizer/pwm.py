# implement PWM class

from .crossfirmarizer import SerialClient

CMD_PWM_CONFIG = 0x56
CMD_PWM_WRITE = 0x57

class PWM:
    def __init__(self, client: SerialClient, pin: int, period_us: int = 20000):
        self.client = client
        self.pin = pin
        self.period_us = period_us
        
        # Configure PWM period
        payload = bytes([self.pin, self.period_us & 0xFF, (self.period_us >> 8) & 0xFF])
        self.client.send_packet(CMD_PWM_CONFIG, payload)

    def write(self, value: int):
        if not (0 <= value <= self.period_us):
            raise ValueError(f"PWM value must be between 0 and {self.period_us}")
        
        # Send the 16-bit duty cycle to STM32
        payload = bytes([self.pin, value & 0xFF, (value >> 8) & 0xFF])
        self.client.send_packet(CMD_PWM_WRITE, payload)
