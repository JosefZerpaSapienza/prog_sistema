// Define message handling and command execution.
//
#include "constants.h"

#ifdef __linux__
  #include <sys/wait.h>
  #define LIST "ls"
  #define SIZE "stat --printf=%s\\n"
#elif defined _WIN32
  // Convert POSIX calls to WIN32 API calls
  #define popen(X, Y) _popen(X, Y)
  #define pclose(X) _pclose(X)
  #define WEXITSTATUS(X) X
  #define LIST "dir"
#endif


// Parses the command provided by command line (client side).
// Returns a dynamically allocated string.
char *parse_command(char **argv, int i) {
  // Allocate space.
  char *cmd = malloc(MSG_BUFFER_SIZE);
  memset(cmd, 0, MSG_BUFFER_SIZE);
  // Check '-'
  if (argv[i][0] != '-') {
    return NULL;
  }
  // Parse command.
  switch(argv[i][1]) {
    case 'l':
      strcat(cmd, "LSF");
      return cmd;      
    case 'e':
      strcat(cmd, "EXEC");
      break;
    case 'u':
      strcat(cmd, "UPLOAD");
      break;
    case 'd':
      strcat(cmd, "DOWNLOAD");
      break;
    case 's':
      strcat(cmd, "SIZE");
      break;
    default:
      return NULL;
  }
  // Append arguments.
  while(argv[++i]) {
    strcat(cmd, " ");
    strcat(cmd, argv[i]);
  }

  return cmd;
}

// Return a dynamically allocated string.
void execute_command(char *msg, char **r_result, char **r_code) {
  // Command to be executed.
  char *cmd = (char *)malloc(MSG_BUFFER_SIZE);
  // Results from the execution.
  char *result = (char *)malloc(MSG_BUFFER_SIZE);
  // Results from the execution.
  char *code = (char *)malloc(4);
  int size = MSG_BUFFER_SIZE;
  memset(cmd, 0, MSG_BUFFER_SIZE);
  memset(result, 0, MSG_BUFFER_SIZE);
  
  // Parse command.
  //printf("Received message: %s. \n", msg);
  char *tag = strtok(msg, " ");
  if (strcmp(tag, "LSF") == 0) {
    strcat(cmd, LIST);  
  } else if (strcmp(tag, "EXEC") == 0) {
    cmd = strtok(NULL, "\0");
  }
  // printf("Command: %s\n", cmd);

  // Execute command.
  FILE *fp;
  char buffer[MSG_BUFFER_SIZE];
  fp = popen(cmd, "r");
  if (fp == NULL) {
    sprintf(result, "Could not popen.");
    sprintf(code, "400");
    return;
  }
  // Read output into result.
  while( (fgets(buffer, MSG_BUFFER_SIZE, fp) != NULL) && 
        (size > strlen(buffer)) ) {
    //printf("%s\n", buffer);
    strcat(result, buffer);
    size -= strlen(buffer);
  }
  // Terminate with \r\n\r\n.
  strcat(result, "\r\n\r\n");
  //printf("%s\n", result);
  // Check exit code.
  int status = WEXITSTATUS(pclose(fp));
  // printf("Return status: %d\n", status);
  if (status == 0) {
    // Success
    sprintf(code, "300");
  } else {
    sprintf(code, "400");
    sprintf(result, "%d", status);
  }

  *r_code = code;
  *r_result = result;
}
