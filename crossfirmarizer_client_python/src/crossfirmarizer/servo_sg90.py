from .pwm import PWM

class ServoSG90(PWM):
    def __init__(self, client, pin):
        super().__init__(client, pin)
        self.period = 20  # 20ms period for SG90
        self.min_val = 1000
        self.max_val = 2000
        self.config()

    def config(self):
        # For SG90, the pulse width typically ranges from 1ms (0 degrees) to 2ms (180 degrees)
        # with a period of 20ms. Since the STM32 timer ARR is set to 19999 for PB10, 
        # the duty cycle corresponds directly to the pulse width in microseconds.
        pass

    def write_angle(self, angle: int):
        if not (0 <= angle <= 180):
            raise ValueError("Angle must be between 0 and 180")
        # Map angle to the calculated 16-bit PWM range (1000 - 2000)
        pwm_value = int(self.min_val + (angle / 180.0) * (self.max_val - self.min_val))
        print(f"Writing angle {angle} degrees -> PWM value {pwm_value}")
        self.write(pwm_value)
