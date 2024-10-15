#include "include/interval.h"

// Initialize a time interval with a specified interval
time_interval_t init_time_interval(double seconds) {
  time_interval_t timer;
  timer.last_time = 0.0;
  timer.interval = seconds;
  return timer;
}

// Check if the time interval has elapsed and reset the timer if it has
bool check_time_interval(time_interval_t *timer) {
  // Get the current time in seconds since the epoch.
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);

  // Calculate the elapsed time in seconds.
  double elapsed_time = current_time.tv_sec + current_time.tv_nsec / 1e9 - timer->last_time;

  // Check if the elapsed time is greater than or equal to the interval.
  if (elapsed_time >= timer->interval) {
    timer->last_time = current_time.tv_sec + current_time.tv_nsec / 1e9;
    return true; // Interval has passed
  } else {
    return false; // Interval has not passed yet
  }
}