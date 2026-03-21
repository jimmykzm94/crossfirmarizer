#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the task scheduler.
 * This sets up a 1ms system tick.
 */
void task_init(void);

/**
 * @brief Returns the number of milliseconds since startup.
 * @return The number of milliseconds since task_init was called.
 */
uint32_t task_get_ms(void);

/**
 * @brief Checks if a systick has occurred since the last check.
 * This function should be called frequently in the main loop.
 * The flag is cleared after this function is called.
 * @return true if a 1ms tick has occurred, false otherwise.
 */
bool task_systick_happened(void);