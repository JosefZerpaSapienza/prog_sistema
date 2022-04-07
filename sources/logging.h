// Provide logging utilities.
//
#include "synchronization.h"
#include "timing.h"

// Log file path.
static char *filename = NULL;
// Critical section to access log file.
static void *log_cs = NULL;

// Set log file.
// Return OK on success,
// INT_ERR when an internal function returned error.
int set_logging(char *path) {
  // Set new parameters.
  filename = path;
  log_cs = create_cs();

  if (log_cs == NULL) {
    printf("Could not create critical section. \n");
    return INT_ERR;
  }

  return OK;
}

// Free dynamically allocated memory for logging.
void stop_logging() {
  if (log_cs != NULL) {
    free(log_cs);
  }
}

// Log message into the given file.
// Return OK on success,
// INT_ERR when an internal function returned error.
int log_request(int thread_id, char *ip, int port, char *tag)
{
  char message[MSG_BUFFER_SIZE];
  char timestamp[SMALL_BUFFER_SIZE];
  if (get_time((char *)&timestamp) != OK) {
    printf("Get_time error. \n");
    return INT_ERR;
  }

  sprintf(message, "%s, %s, %d, %s, t_id: %d \n", 
	timestamp, ip, port, tag, thread_id);

  // Check if log file is set.
  if(filename == NULL) {
    printf(" ### LOG FILE NOT SET ### \n");
    printf("%s", message);    
    return INT_ERR;
  } else {
    // Write to log file.
    if (enter_cs(log_cs) != OK) {
      return INT_ERR;
    }
    FILE *file = fopen(filename, "a");
    // Check file opening.
    if (file == NULL) {
   	  perror("Could not open log file.");
   	  return INT_ERR;
    }
    fwrite(message, 1, strlen(message), file);
    fclose(file);
    if (leave_cs(log_cs) != OK) {
    	printf("Could not leave cs. \n");
    }
  }

  return OK;
}

