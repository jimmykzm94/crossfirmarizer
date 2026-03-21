/*
 * timer.h
 *
 *  Created on: Mar 15, 2026
 *      Author: jimmy
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "pins.h"

typedef enum
{
    TIMER_1,
    TIMER_2,
    TIMER_3,
    TIMER_4,
    TIMER_5
} timer_type_t;

typedef void (*TimerCallback)(uint32_t);

void timer_capture_init(timer_type_t timer_type, pin_t pin, uint32_t timeout, TimerCallback callback);
void timer_capture_start(timer_type_t timer_type);
timer_type_t timer_get_timer_for_pin(pin_t pin);

void timer_delay_init(timer_type_t timer_type, TimerCallback callback);
void timer_delay_start_us(uint32_t us);


#endif /* INC_TIMER_H_ */
