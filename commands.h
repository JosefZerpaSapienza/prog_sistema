// Define message handling and command execution.
//
#include "constants.h"

// Return a dynamically allocated string.
char *execute_command(char *msg, int len) {
  char *response = (char *)malloc(MSG_BUFFER_SIZE);
  int recv_bytes;
  memset(response, 0, MSG_BUFFER_SIZE);

 
  // Parse command.
  printf("Received message: %s. \n", msg);

  // Execute command.
  

  sprintf(response, "Hello from the server!"); // dummy

  return response;
}
