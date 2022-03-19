// Define message handling and command execution.
//
#include "constants.h"
#define TERMINATION_STRING "\r\n\r\n"

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
int parse_command(char **argv, int i, char **command) {
  char *cmd = *command;
  memset(cmd, 0, MSG_BUFFER_SIZE);
  // Check '-'
  if (argv[i][0] != '-') {
    return PROTO_ERR;
  }
  // Parse command.
  switch(argv[i][1]) {
    case 'l':
      strcat(cmd, "LSF");
      return OK;      
    case 'e':
      strcat(cmd, "EXEC");
      break;
    case 'u':
      strcat(cmd, "UPLOAD");
      // TODO: Implement.
      break;
    case 'd':
      strcat(cmd, "DOWNLOAD");
      // TODO: Implement.
      break;
    case 's':
      strcat(cmd, "SIZE");
      // TODO: Implement.
      break;
    default:
      return PARAM_ERR;
  }

  // Append arguments.
  while(argv[++i]) {
    strcat(cmd, " ");
    strcat(cmd, argv[i]);
  }

  return OK;      
}

// Parse command from msg, and place output in e_result.
// Also set e_code accordingly.
// Return:
// OK on success,
// COMM_ERR when parsing and unsupported command,
// INT_ERR when cannot popen. Launch perror() to print details.
int execute_command(char *msg, char **e_result, char **e_code) {
  // Set local pointer to external e_result..
  char *result = *e_result;
  // Result code from the execution.
  char *code = *e_code;
  // Command to be executed.
  char cmd[MSG_BUFFER_SIZE];
  int size = MSG_BUFFER_SIZE;
  memset(cmd, 0, MSG_BUFFER_SIZE);
  memset(result, 0, MSG_BUFFER_SIZE);
  
  // Parse msg into cmd.
  char *tag = strtok(msg, " ");
  if (strcmp(tag, "LSF") == 0) {
    strcat(cmd, LIST);  
  } else if (strcmp(tag, "EXEC") == 0) {
    strcat(cmd, strtok(NULL, "\0"));
  } else {
    // Command not supported.
    sprintf(result, "Command not supported.");
    sprintf(code, "400");
    return PROTO_ERR;

  }
  // TODO: implement UPLOAD, DOWNLOAD, SIZE.

  //  printf("Command: %s\n", cmd); //DBG

  // Execute command with popen.
  char buffer[MSG_BUFFER_SIZE];
  FILE *fp = popen(cmd, "r");
  // Check popen error.
  if (fp == NULL) {
    sprintf(result, "Could not popen.");
    sprintf(code, "400");
    return INT_ERR;
  }

  // Read popen output into result.
  while( (fgets(buffer, MSG_BUFFER_SIZE, fp) != NULL) && 
        (size > strlen(buffer)) ) {
    strcat(result, buffer);
    size -= strlen(buffer);
  }
  // Append termination.
  strcat(result, TERMINATION_STRING);

  //printf("%s\n", result);

  // Check exit code.
  int status = WEXITSTATUS(pclose(fp));
  if (status == 0) {
    // Success
    sprintf(code, "300");
    return OK;
  } else {
    // Exited with error.
    sprintf(result, "%d", status);
    sprintf(code, "400");
    return COMM_ERR;
  }
}
