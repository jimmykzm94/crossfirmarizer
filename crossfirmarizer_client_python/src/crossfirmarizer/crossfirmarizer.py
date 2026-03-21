import serial
import threading
import time
import queue
from .serial_packet import create_packet, parse_packet

# command id
CMD_PIN_MODE = 0x01
CMD_SETTER = 0x02
CMD_GETTER = 0x03
CMD_GPIO_LISTEN = 0x05
CMD_I2C_CONFIG = 0x10
CMD_I2C_WRITE = 0x11
CMD_I2C_READ = 0x12
CMD_ULTRASONIC_READ = 0x70
CMD_ULTRASONIC_CONFIG = 0x71
CMD_HELLO = 0x99

class SerialClient:
    def __init__(self, port: str):
        self.ser = serial.Serial(port, 115200, timeout=1)
        self.pin_states = {}
        self._raw_packet_queue = queue.Queue()
        self._sync_cmd_response_queue = queue.Queue(maxsize=1)
        self._lock = threading.Lock()
        self._write_lock = threading.Lock()
        self._listener_active = False
        self._reader_thread = None
        self._parser_thread = None
        self.start_listener()

    def send_packet(self, cmd: int, payload: bytes):
        with self._write_lock:
            packet = create_packet(cmd, payload)
            self.ser.write(packet)

    def close(self):
        self.stop_listener()
        self.ser.close()

    def hello(self):
        self.send_packet(CMD_HELLO, b'')

    def set_gpio(self, pin: int, mode: int):
        payload = bytes([pin, mode])
        self.send_packet(CMD_PIN_MODE, payload)

    def digital_write(self, pin: int, value: int):
        payload = bytes([pin, value])
        self.send_packet(CMD_SETTER, payload)

    def pwm_write(self, pin: int, value: int):
        # send value as 16-bit little endian
        payload = bytes([pin, value & 0xFF, (value >> 8) & 0xFF])
        self.send_packet(CMD_SETTER, payload)

    # ⚠️ I am thinking want to keep this, since gpio listening will corrupt this method.
    def digital_read(self, pin: int):
        with self._lock:
            # Clear any stale responses from the queue
            while not self._sync_cmd_response_queue.empty():
                self._sync_cmd_response_queue.get_nowait()

            payload = bytes([pin])
            self.send_packet(CMD_GETTER, payload)

            try:
                # Wait for the parser thread to put the response in the queue
                cmd, payload, checksum = self._sync_cmd_response_queue.get(timeout=2.0)
                if cmd == CMD_GETTER and len(payload) == 2 and payload[0] == pin:
                    return payload[1]  # Return the value
                else:
                    print(f"Warning: Received unexpected sync response for pin {pin}.")
                    return None
            except queue.Empty:
                print(f"Failed to get response for digital_read on pin {pin} (timeout).")
                return None

    def read(self, pin: int):
        return self.pin_states.get(pin, 0)

    def i2c_config(self, bus: int, sda_pin: int, scl_pin: int):
        """Configure an I2C bus with specific SDA and SCL pins."""
        payload = bytes([bus, sda_pin, scl_pin])
        self.send_packet(CMD_I2C_CONFIG, payload)

    def i2c_write(self, bus: int, address: int, data: bytes):
        """Write an array of bytes to a specific I2C address on a given bus."""
        payload = bytes([bus, address]) + bytes(data)
        self.send_packet(CMD_I2C_WRITE, payload)

    def i2c_read(self, bus: int, address: int, size: int):
        """Read a specified number of bytes from an I2C address."""
        with self._lock:
            # Clear any stale responses from the queue
            while not self._sync_cmd_response_queue.empty():
                self._sync_cmd_response_queue.get_nowait()

            payload = bytes([bus, address, size])
            self.send_packet(CMD_I2C_READ, payload)

            try:
                # Wait for the parser thread to put the response in the queue
                cmd, resp_payload, checksum = self._sync_cmd_response_queue.get(timeout=2.0)
                if cmd == CMD_I2C_READ and len(resp_payload) >= 2 and resp_payload[0] == bus and resp_payload[1] == address:
                    return resp_payload[2:]  # Return the requested data
                else:
                    print(f"Warning: Received unexpected sync response for i2c_read on bus {bus}, address {address}.")
                    return None
            except queue.Empty:
                print(f"Failed to get response for i2c_read on bus {bus}, address {address} (timeout).")
                return None

    def ultrasonic_config(self, trig, ehco, interval):
        payload = bytes([trig, ehco, interval & 0xFF, (interval >> 8) & 0xFF])
        self.send_packet(CMD_ULTRASONIC_CONFIG, payload)

    def ultrasonic_read(self):
        with self._lock:
            while not self._sync_cmd_response_queue.empty():
                self._sync_cmd_response_queue.get_nowait()

            self.send_packet(CMD_ULTRASONIC_READ, b'')

            try:
                cmd, payload, checksum = self._sync_cmd_response_queue.get(timeout=2.0)
                if cmd == CMD_ULTRASONIC_READ and len(payload) == 4:
                    distance_cm_x100 = int.from_bytes(payload, 'big')
                    return distance_cm_x100 / 100.0
                else:
                    print("Warning: Received unexpected sync response for ultrasonic_read.")
                    return None
            except queue.Empty:
                print("Failed to get response for ultrasonic_read (timeout).")
                return None

    def gpio_listen(self, pin: int, enable: bool):
        """Enable or disable asynchronous listening for a specific pin."""
        payload = bytes([pin, 1 if enable else 0])
        self.send_packet(CMD_GPIO_LISTEN, payload)
        if enable:
            if pin not in self.pin_states:
                self.pin_states[pin] = 0  # Initialize state for this pin
        else:
            if pin in self.pin_states:
                del self.pin_states[pin]  # Remove state tracking for this pin

    def start_listener(self):
        """Start the background threads to read and parse incoming packets."""
        if not self._listener_active:
            self._listener_active = True
            self._reader_thread = threading.Thread(target=self._reader_worker, daemon=True)
            self._parser_thread = threading.Thread(target=self._parser_worker, daemon=True)
            self._reader_thread.start()
            self._parser_thread.start()

    def stop_listener(self):
        """Stop the background listener threads."""
        if not self._listener_active:
            return

        # Gracefully disable listening on all active pins
        for pin in list(self.pin_states.keys()):
            self.gpio_listen(pin, False)
        time.sleep(0.1) # Allow time for commands to be sent

        self._listener_active = False

        if self._parser_thread:
            self._raw_packet_queue.put(None) # Sentinel to unblock parser

        if self._reader_thread:
            self._reader_thread.join(timeout=1.0)
        if self._parser_thread:
            self._parser_thread.join(timeout=1.0)

    def _reader_worker(self):
        """Background worker that reads from serial and puts complete packets into a queue."""
        packet_buffer = bytearray()
        while self._listener_active:
            try:
                if self.ser.in_waiting > 0:
                    byte = self.ser.read(1)
                    if byte:
                        packet_buffer.extend(byte)
                        if byte == b'\x00':
                            self._raw_packet_queue.put(bytes(packet_buffer))
                            packet_buffer.clear()
                else:
                    time.sleep(0.005) # Prevent busy-waiting
            except serial.SerialException:
                self._listener_active = False
                break

    def _parser_worker(self):
        """Background worker that parses packets from the queue and dispatches them."""
        while self._listener_active:
            try:
                raw_packet = self._raw_packet_queue.get(timeout=1.0)
                if raw_packet is None: continue

                cmd, payload, checksum = parse_packet(raw_packet)

                if cmd == CMD_GPIO_LISTEN and len(payload) == 2:
                    pin, value = payload[0], payload[1]
                    self.pin_states[pin] = value
                elif cmd in (CMD_GETTER, CMD_I2C_READ, CMD_ULTRASONIC_READ):
                    # This is a response for a synchronous read command
                    if not self._sync_cmd_response_queue.full():
                        self._sync_cmd_response_queue.put((cmd, payload, checksum))
                elif cmd == CMD_HELLO:
                    print(f'CMD_HELLO response: {payload.decode()}')

            except (queue.Empty, ValueError):
                continue

class ArduinoClient(SerialClient):
    # When connecting Arduino port, it will trigger a reset so delay 2s
    def __init__(self, port: str):
        super().__init__(port)
        time.sleep(2)

class PinMode:
    INPUT = 0
    OUTPUT = 1
    ANALOG = 2

# Example usage:
if __name__ == "__main__":
    client = SerialClient('/dev/tty.usbmodem113403')
    try:
        while True:
            client.hello()
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        client.close()