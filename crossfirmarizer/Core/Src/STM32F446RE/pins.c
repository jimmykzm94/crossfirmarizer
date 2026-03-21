#include "pins.h"
#include <stddef.h> // Required for NULL

#ifdef TARGET_STM32F446RE

const pin_map_t pins[PIN_MAX_COUNT] = {
    // CN7 pins (1-indexed on board; index 0 is left null)
    [CN7_PIN_0] = {NULL, 0},
    [CN7_PIN_1] = {GPIOC, 10},
    [CN7_PIN_2] = {GPIOC, 11},
    [CN7_PIN_3] = {GPIOC, 12},
    [CN7_PIN_4] = {GPIOD, 2},
    [CN7_PIN_5] = {NULL, 0},  // VDD
    [CN7_PIN_6] = {NULL, 0},  // E5V
    [CN7_PIN_7] = {NULL, 0},  // BOOT0
    [CN7_PIN_8] = {NULL, 0},  // GND
    [CN7_PIN_9] = {NULL, 0},  // NC
    [CN7_PIN_10] = {NULL, 0}, // NC
    [CN7_PIN_11] = {NULL, 0}, // NC
    [CN7_PIN_12] = {NULL, 0}, // IOREF
    [CN7_PIN_13] = {GPIOA, 13},
    [CN7_PIN_14] = {NULL, 0}, // RESET
    [CN7_PIN_15] = {GPIOA, 14},
    [CN7_PIN_16] = {NULL, 0}, // +3V3
    [CN7_PIN_17] = {GPIOA, 15},
    [CN7_PIN_18] = {NULL, 0}, // +5V
    [CN7_PIN_19] = {NULL, 0}, // GND
    [CN7_PIN_20] = {NULL, 0}, // GND
    [CN7_PIN_21] = {GPIOB, 7},
    [CN7_PIN_22] = {NULL, 0}, // GND
    [CN7_PIN_23] = {GPIOC, 13},
    [CN7_PIN_24] = {NULL, 0}, // VIN
    [CN7_PIN_25] = {GPIOC, 14},
    [CN7_PIN_26] = {NULL, 0}, // NC
    [CN7_PIN_27] = {GPIOC, 15},
    [CN7_PIN_28] = {GPIOA, 0},
    [CN7_PIN_29] = {GPIOH, 0},
    [CN7_PIN_30] = {GPIOA, 1},
    [CN7_PIN_31] = {GPIOH, 1},
    [CN7_PIN_32] = {GPIOA, 4},
    [CN7_PIN_33] = {NULL, 0}, // VBAT
    [CN7_PIN_34] = {GPIOB, 0},
    [CN7_PIN_35] = {GPIOC, 2},
    [CN7_PIN_36] = {GPIOC, 1},
    [CN7_PIN_37] = {GPIOC, 3},
    [CN7_PIN_38] = {GPIOC, 0},

    // CN10 pins (1-indexed on board; index 0 is left null)
    [CN10_PIN_0] = {NULL, 0},
    [CN10_PIN_1] = {GPIOC, 9},
    [CN10_PIN_2] = {GPIOC, 8},
    [CN10_PIN_3] = {GPIOB, 8},
    [CN10_PIN_4] = {GPIOC, 6},
    [CN10_PIN_5] = {GPIOB, 9},
    [CN10_PIN_6] = {GPIOC, 5},
    [CN10_PIN_7] = {NULL, 0},  // AVDD
    [CN10_PIN_8] = {NULL, 0},  // U5V
    [CN10_PIN_9] = {NULL, 0},  // GND
    [CN10_PIN_10] = {NULL, 0}, // NC
    [CN10_PIN_11] = {GPIOA, 5},
    [CN10_PIN_12] = {GPIOA, 12},
    [CN10_PIN_13] = {GPIOA, 6},
    [CN10_PIN_14] = {GPIOA, 11},
    [CN10_PIN_15] = {GPIOA, 7},
    [CN10_PIN_16] = {GPIOB, 12},
    [CN10_PIN_17] = {GPIOB, 6},
    [CN10_PIN_18] = {NULL, 0}, // NC
    [CN10_PIN_19] = {GPIOC, 7},
    [CN10_PIN_20] = {NULL, 0}, // GND
    [CN10_PIN_21] = {GPIOA, 9},
    [CN10_PIN_22] = {GPIOB, 2},
    [CN10_PIN_23] = {GPIOA, 8},
    [CN10_PIN_24] = {GPIOB, 1},
    [CN10_PIN_25] = {GPIOB, 10},
    [CN10_PIN_26] = {GPIOB, 15},
    [CN10_PIN_27] = {GPIOB, 4},
    [CN10_PIN_28] = {GPIOB, 14},
    [CN10_PIN_29] = {GPIOB, 5},
    [CN10_PIN_30] = {GPIOB, 13},
    [CN10_PIN_31] = {GPIOB, 3},
    [CN10_PIN_32] = {NULL, 0}, // AGND
    [CN10_PIN_33] = {GPIOA, 10},
    [CN10_PIN_34] = {GPIOC, 4},
    [CN10_PIN_35] = {GPIOA, 2},
    [CN10_PIN_36] = {NULL, 0}, // NC
    [CN10_PIN_37] = {GPIOA, 3},
    [CN10_PIN_38] = {NULL, 0}, // NC

    // Arduino Analog Pins
    [A0] = {GPIOA, 0},
    [A1] = {GPIOA, 1},
    [A2] = {GPIOA, 4},
    [A3] = {GPIOB, 0},
    [A4] = {GPIOC, 1},
    [A5] = {GPIOC, 0},

    // Arduino Digital Pins
    [D0] = {GPIOA, 3},
    [D1] = {GPIOA, 2},
    [D2] = {GPIOA, 10},
    [D3] = {GPIOB, 3},
    [D4] = {GPIOB, 5},
    [D5] = {GPIOB, 4},
    [D6] = {GPIOB, 10},
    [D7] = {GPIOA, 8},
    [D8] = {GPIOA, 9},
    [D9] = {GPIOC, 7},
    [D10] = {GPIOB, 6},
    [D11] = {GPIOA, 7},
    [D12] = {GPIOA, 6},
    [D13] = {GPIOA, 5},
    [D14] = {GPIOB, 9},
    [D15] = {GPIOB, 8},

    // Helpful pins
    [PIN_LED] = {GPIOA, 5},    // Standard D13 / PA5 User LED
    [PIN_BUTTON] = {GPIOC, 13} // Standard User Button (B1)
};

#endif