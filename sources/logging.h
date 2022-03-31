// Provide logging utilities.
//
#include "synchronization.h"
#include "timing.h"

// Log file path.
static char *log_file = NULL;
// Critical section to access log file.
static void *log_cs = NULL;

// Set log file.
void set_log_file(char *path) {
  log_file = path;
  log_cs = create_cs();
}

// Log message into the given file.
void log_request(int thread_id, char *ip, int port, char *tag) 
{
  char message[MSG_BUFFER_SIZE];
  char *timestamp = get_time();
  sprintf(message, "%s, %s, %d, %s, t_id: %d \n", 
	timestamp, ip, port, tag, thread_id);
  free(timestamp);

  if(log_file == NULL) {
    printf(" ### LOG FILE NOT SET ### \n");
    printf("%s", message);    
  } else {
    // Write to log file.
    enter_cs(log_cs);
    FILE *file = fopen(log_file, "a");
    fwrite(message, 1, strlen(message), file);
    fclose(file);
    leave_cs(log_cs);
  }
}

