#ifndef __INTERVAL_H__
#define __INTERVAL_H__

#include <time.h>
#include <stdbool.h>

typedef struct time_interval_t {
  double last_time;
  double interval;
} time_interval_t;

// Initialize a time interval with a specified interval
time_interval_t init_time_interval(double seconds);

// Check if the time interval has elapsed and reset the timer if it has
bool check_time_interval(time_interval_t *timer);

#endif