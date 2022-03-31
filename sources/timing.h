// Provide timing utilities.
//
#include <time.h>

// Return a printable timestamp.
char *get_time() {
  time_t ltime;
  struct tm result;
  char *timestamp = malloc(SMALL_BUFFER_SIZE);

  ltime = time(NULL);
#ifdef __linux__
  localtime_r(&ltime, &result);
  asctime_r(&result, timestamp);
#elif defined _WIN32
  timestamp = asctime(localtime(&ltime));
#endif

  timestamp = strtok(timestamp, "\n");

  return timestamp;
}
