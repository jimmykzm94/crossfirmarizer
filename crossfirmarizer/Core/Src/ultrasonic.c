/*
 * ultrasonic.c
 *
 *  Created on: Mar 15, 2026
 *      Author: jimmy
 */

#include "ultrasonic.h"
#include "timer.h"
#include "gpio.h"
#include <stddef.h>
#include <stdbool.h>
#include "command.h"

#ifdef ULTRASONIC_SENSOR_ENABLED

typedef enum {
    IDLE,
    READY,
    TRIGGERING,
    WAITING_FOR_ECHO
} ultrasonic_state_t;

static volatile ultrasonic_state_t state = IDLE;
static pin_t trig_pin;
static pin_t echo_pin;
static timer_type_t capture_timer;
static volatile float last_reading;

static void ultrasonic_trigger_finished(uint32_t ignored)
{
    // 10us have passed, end the trigger pulse
    gpio_write(trig_pin, 0);

    // Start listening for the echo
    timer_capture_start(capture_timer);
    state = WAITING_FOR_ECHO;
}

static void ultrasonic_echo_callback(uint32_t time_us)
{
    last_reading = (time_us * 0.0343f) / 2.0f;
    state = READY;
}

void ultrasonic_init(pin_t trig, pin_t echo, uint16_t interval)
{
    trig_pin = trig;
    echo_pin = echo; 
    capture_timer = timer_get_timer_for_pin(echo);
    // uint8_t data = (echo == 55) ? 'y' : 'n';
    // command_send(CMD_HELLO, &data, 1);

    // Configure trigger pin as output
    gpio_init(trig_pin, PIN_MODE_OUTPUT);

    // Init TIM2 for the 10us non-blocking trigger delay
    timer_delay_init(TIMER_2, ultrasonic_trigger_finished);

    // Init the capture timer for the echo measurement.
    timer_capture_init(capture_timer, echo, interval * 1000, ultrasonic_echo_callback);

    // Set READY state
    state = READY;
}

void ultrasonic_deinit(void)
{
    // Deinit
    state = IDLE;
    // gpio_deinit(trig_pin);
    // timer_init(TIM2_DELAY, 0, NULL);
    // timer_init(TIM1_INPUT_CAPTURE, 0, NULL);
}

void ultrasonic_read_task(void) {
    // Only start a new measurement if the previous one is complete
    if (state == READY)
    {
        state = TRIGGERING;

        // Start 10us trigger pulse
        gpio_write(trig_pin, 1);
        timer_delay_start_us(10);
    }
}

float ultrasonic_get_last_reading(void)
{
    return last_reading;
}

#endif /* ULTRASONIC_SENSOR_ENABLED */
