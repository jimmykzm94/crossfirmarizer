# 🌷 CrossFirmarizer 🌷
![crossfirmarizer_banner.png](crossfirmarizer_banner.png)

CrossFirmarizer is a lightweight, high-performance alternative to Firmata for controlling microcontrollers from a host computer—without sacrificing low-level control or portability. It exists to bridge high-level experimentation and low-level embedded control—without forcing you to choose between them.

Unlike Firmata, it allows seamless integration of vendor HAL drivers and custom bare-metal logic without breaking the communication layer.

> **⚠️ Prototype-Focused:** CrossFirmarizer is designed for rapid development and experimentation. It is not optimized for production reliability, safety-critical systems, or long-term deployment.

## Architecture Overview

CrossFirmarizer consists of two main components:

- **Firmware (MCU side)**: Handles hardware access and interprets serial commands.
- **Client (Host side)**: Sends commands over serial using a standardized packet protocol.

Communication Flow:
`Client → Serial Packet → Firmware → HAL → Hardware`

## Features

- **Client-Side I/O Control:** Easily read and control microcontroller pins directly from your computer.
- **Hardware Agnostic:** Out-of-the-box support for multiple MCU architectures.
- **Extensible Architecture:** Seamlessly integrate vendor HAL drivers or bare-metal C code.
- **Low-Latency Communication:** Minimal-overhead serial protocol designed for fast command-response cycles during development.
- **Cross-Language Ready:** Built on a standardized serial packet protocol, making it easy to port the client library to languages beyond Python.

## Supported Capabilities

- **GPIO:** Digital read/write, pin mode control
- **PWM:** Basic analog output
- **I2C:** Bus configuration, read, and write
- **Ultrasonic Sensor:** Dedicated module for HC-SR04 style sensors

> Feature availability may vary by MCU target.

## Comparison

| Feature                  | CrossFirmarizer | Firmata |
|--------------------------|-----------------|---------|
| Custom Firmware Logic    | ✅ Yes          | ❌ No   |
| Custom HAL/Bare-Metal    | ✅ Yes          | ❌ No   |
| Cross-MCU Design         | ✅ Yes          | ⚠️ Limited |
| Performance Focus        | ✅ Yes          | ❌ No   |
| Language Extensible      | ✅ Yes          | ⚠️ Limited |

## Installation

```bash
pip install crossfirmarizer-x.y.z-py3-none-any.whl
```

## Getting Started

### 1. Flash Firmware
Build the firmware for your target microcontroller and flash it. See the "Building from Source" section for detailed instructions.
> STM32 users: You can use an IDE like STM32CubeIDE (see build instructions below) for a simpler, integrated setup.

### 2. Run Example
The following example shows how to blink the built-in LED.

**Choose your client:**
- `SerialClient` → For STM32 and other generic boards.
- `ArduinoClient` → For ATmega boards (e.g., Arduino Mega), as it handles the auto-reset delay.

```python
# Use the client and target appropriate for your board
from crossfirmarizer import SerialClient, PinMode # or ArduinoClient
from crossfirmarizer.targets import STM32F446RE # or ATmega2560
import time

if __name__ == "__main__":
    # macOS/Linux: /dev/tty.* | Windows: COMx
    client = SerialClient('/dev/tty.usbmodemXXXX') # or ArduinoClient
    
    # Set LED pin to OUTPUT mode
    client.set_gpio(STM32F446RE.LED, PinMode.OUTPUT)
    
    # Blink the LED
    try:
        while True:
            client.digital_write(STM32F446RE.LED, 1)
            time.sleep(0.5)
            client.digital_write(STM32F446RE.LED, 0)
            time.sleep(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        print("Cleaning up and closing client.")
        client.digital_write(STM32F446RE.LED, 0)
        client.close()
```

## Serial Communication

### Physical Layer
- **Interface:** UART
- **Baud Rate:** 115200
- **Configuration:** 8N1 (8 data bits, no parity, 1 stop bit)
- **Performance:** The firmware leverages DMA (Direct Memory Access) for UART communication on supported targets like the STM32 to ensure high throughput and low CPU overhead. Other targets may use interrupt-driven or polled UART.

### Packet Structure
For advanced users and developers porting the client to other languages, understanding the packet structure is key.

Each raw message packet has the structure: `[CMD][LEN][PAYLOAD][CHECKSUM]`
- **CMD (1 byte):** The command identifier (e.g., `0x01` for Pin Mode).
- **LEN (1 byte):** Length of the payload.
- **PAYLOAD (0-32 bytes):** The data associated with the command.
- **CHECKSUM (1 byte):** A simple XOR checksum of all bytes from CMD to PAYLOAD.

To ensure data integrity and handle `0x00` bytes within the payload, this raw packet is encoded using **Consistent Overhead Byte Stuffing (COBS)**. A final `0x00` byte is then appended as a frame delimiter.

The final transmitted message is: `[COBS-ENCODED-PACKET][0x00]`

## Building from Source

### Python Client
1. Install build dependencies:
```bash
pip install build
```
2. Build the package:
```bash
python -m build crossfirmarizer_client_python
```

### ATmega2560 Firmware
1. Build:
```bash
make -f crossfirmarizer/crossfirmarizer_ATmega2560.mk all
```
2. Flash:
```bash
make -f crossfirmarizer/crossfirmarizer_ATmega2560.mk upload PORT=/dev/your_port
```

### STM32F446RE Firmware

**Using Makefile:**
1. Build: `make -f crossfirmarizer/crossfirmarizer_STM32F446xx.mk all`
2. Flash with a tool like `st-flash`:
```bash
st-flash --format hex write crossfirmarizer_STM32F446RE.hex
```

**Using STM32CubeIDE:**
1. Create a new project from existing C code, pointing to the `crossfirmarizer` directory.
2. Configure the toolchain (`arm-none-eabi-gcc`) and build settings.
3. Build and flash directly from the IDE.

## For Developers

This section provides guidance for developers who want to extend CrossFirmarizer to support new hardware, client languages, or features.

### Firmware (FW)
Adding a new MCU typically involves two steps:

**Step 1: Define Pins**
Create a `pins.h` for your target MCU in a new folder under `crossfirmarizer/Core/Inc/`.
This file must define:
- A `pin_t` enum listing all available pins and helpers (e.g., `PIN_LED`).
- A `pin_map_t` struct to map the logical pins to their physical hardware registers/ports.

**Step 2: Implement HAL**
Implement the hardware abstraction layer (HAL) for your MCU. This means writing the C source files for GPIO, I2C, PWM, etc., that fulfill the contracts defined in the header files in `crossfirmarizer/Core/Inc/`.

### Client Software (SW)

**Step 3: Define Pins**
In the client library, define the pin mappings for the new hardware. For the Python client, create a new class in `crossfirmarizer_client_python/src/crossfirmarizer/targets/` matching the `pin_t` enum from the firmware.

## CI / Release Pipeline

This project uses CircleCI for continuous integration, defined in `.circleci/config.yml`. When a new version is pushed to a `release/*` branch, the pipeline automatically builds the Python client, compiles the firmware for all targets, and attaches the artifacts to a new GitHub Release.
