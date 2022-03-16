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
void *parse_command(char **argv, int i, char **command) {
  char *cmd = *command;
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
}

// Return a dynamically allocated string.
void execute_command(char *msg, char **r_result, char **r_code) {
  // Results from the execution.
  char *result = *r_result;
  // Results from the execution.
  char *code = *r_code;
  // Command to be executed.
  char cmd[MSG_BUFFER_SIZE];
  int size = MSG_BUFFER_SIZE;
  memset(cmd, 0, MSG_BUFFER_SIZE);
  memset(result, 0, MSG_BUFFER_SIZE);
  
  // Parse command.
  //printf("Received message: %s. \n", msg);
  char *tag = strtok(msg, " ");
  if (strcmp(tag, "LSF") == 0) {
    strcat(cmd, LIST);  
  } else if (strcmp(tag, "EXEC") == 0) {
    strcat(cmd, strtok(NULL, "\0"));
  }
  // printf("Command: %s\n", cmd);

  // Execute command with popen.
  char buffer[MSG_BUFFER_SIZE];
  FILE *fp;
  fp = popen(cmd, "r");
  if (fp == NULL) {
    sprintf(result, "Could not popen.");
    sprintf(code, "400");
    return;
  }

  // Read output into result.
  while( (fgets(buffer, MSG_BUFFER_SIZE, fp) != NULL) && 
        (size > strlen(buffer)) ) {
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
}
