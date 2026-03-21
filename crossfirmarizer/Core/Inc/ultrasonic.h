/*
 * ultrasonic.h
 *
 *  Created on: Mar 15, 2026
 *      Author: jimmy
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#ifdef ULTRASONIC_SENSOR_ENABLED

#include "pins.h"
#include <stdint.h>

// Timeout for the sensor echo in microseconds. 30ms is a safe value.
// #define ULTRASONIC_TIMEOUT 60000U

void ultrasonic_init(pin_t trig, pin_t echo, uint16_t interval);
void ultrasonic_deinit(void);
void ultrasonic_read_task(void);
float ultrasonic_get_last_reading(void);

#endif /* ULTRASONIC_SENSOR_ENABLED */

#endif /* INC_ULTRASONIC_H_ */
