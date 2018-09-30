#ifndef _CORE_HAL_NATIVE_TIMER_H_
#define _CORE_HAL_NATIVE_TIMER_H_
#include <stdint.h>

/**
 * Number of nanoseconds after the kernel starts
 */
uint64_t get_time_nano();

/**
 * Schedule a new alarm in nano seconds.
 * At the moment of the timer, the method handle_alarm will be called.
 */
bool set_alarm(uint64_t sleep_time_in_nano);

void handle_alarm();

#endif