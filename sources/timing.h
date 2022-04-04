// Provide timing utilities.
//
#include <time.h>

// Return a printable timestamp.
// On failure: NULL.
int get_time(char **e_timestamp) {
  char *timestamp = *e_timestamp;
  time_t ltime = time(NULL);

  // Check return value.
  if (time == -1) {
    perror("Time error.");
    return INT_ERR;
  }

#ifdef __linux__
  struct tm result;
  if (localtime_r(&ltime, &result) == NULL) {
    perror("Time parsing error.");
    return INT_ERR;
  }
  if (asctime_r(&result, timestamp) == NULL) {
    perror("Time parsing error.");
    return INT_ERR;
  }
#elif defined _WIN32
  struct tm *lotime = localtime(&ltime);
  if (lotime == NULL) {
    printf("Time parsing error. \n");
    return INT_ERR;
  }
  timestamp = asctime(lotime);
  if (timestamp == NULL) {
    printf("Time parsing error. \n");
    return INT_ERR;
  }
#endif

  timestamp = strtok(timestamp, "\n");

  return OK;
}
