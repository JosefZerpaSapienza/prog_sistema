// Define message handling and command execution.
//
#include "constants.h"
#define TERMINATION_STRING "\r\n\r\n"

#ifdef __linux__
  #include <sys/wait.h>
  #define LIST "ls"
  #define SIZE "stat --printf=%s\\\n "
#elif defined _WIN32
  // Convert POSIX calls to WIN32 API calls
  #define popen(X, Y) _popen(X, Y)
  #define pclose(X) _pclose(X)
  #define WEXITSTATUS(X) X
  #define LIST "dir"
  #define SIZE "FORFILES  /C \"cmd /c echo @fsize\" /M "
#endif

// Execute cmd as a system call.
// Writes output into *e_result, and exit code into *e_code.
int exec(char *cmd, char **e_result, char **e_code) {
  char *result = *e_result;
  char *code = *e_code;
  char buffer[MSG_BUFFER_SIZE];
  int size = MSG_BUFFER_SIZE;
  // Execute command with popen.
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

  // TODO: Check termination of output (eof)
  // and send response accordingly (eg. code 300).

  // Append termination.
  strcat(result, TERMINATION_STRING);

  //  printf("Result: \n%s \n.\n", result); //DBG

#ifdef _WIN32
  // Windows printf feof.
  if(feof(fp)) {
    printf("End of file. \n");
  } else {
    printf("Not end of file.\n");
  }
#endif
  // Check exit code.
  int status = WEXITSTATUS(pclose(fp));
  if (status == 0) {
    // Success
    sprintf(code, "300");
    return OK;
  } else {
    // Exited with error.
    printf("Returned code: %d.\n", status);
    sprintf(result, "%d", status);
    sprintf(code, "400");
    return COMM_ERR;
  }
}

// Exec SIZE command to get file size.
void get_file_size (char *filename, char **size) {
  char cmd[MSG_BUFFER_SIZE];
  char response[MSG_BUFFER_SIZE];
  char code[MSG_BUFFER_SIZE];
  
  sprintf(cmd, "%s %s", SIZE, filename);
  exec(cmd, (char **) &response, (char **) &code);
  sprintf(*size, "%s", strtok(response, "\n")); 

  return;
}

// (Client side) Handle response from command execution.
int handle_response(char *cmd, int conn, char **response, char **code) {
  // Get type of command sent.
  char *tag = strtok(cmd, " ");

  // Receive code.
  int recv_bytes;
  if((recv_bytes = recv(conn, *code, 3, 0) ) < 0) {
    return CONN_ERR;
  }
    
  // Handle response accordingly.
  if ( (strcmp(tag, "LIST") == 0) 
        || (strcmp(tag, "SIZE") == 0)
	|| (strcmp(tag, "EXEC") == 0) ) {
    // Check code.
    if (strcmp(*code, "300") == 0) {
      // Receive results.
      if((recv_bytes = recv(conn, *response, MSG_BUFFER_SIZE, 0) ) < 0) {
        return CONN_ERR;
      }
      // Execution succesful.
      return OK;
    } else {
      // Returned error code.
      return PROTO_ERR;
    }
  } else if (strcmp(tag, "DOWNLOAD") == 0) {
    // Check code.
    if (strcmp(*code, "200") == 0) {
      // Send file.
      char *filename = strtok(NULL, " ");
      FILE *file = fopen(filename, "r");
      // Check error.
      if (file == NULL) {
        return INT_ERR;
      }

      char buffer[MSG_BUFFER_SIZE];
      int chars_read = MSG_BUFFER_SIZE;
      // Read chars.
      // TODO: When filesize is modulo MSG_BUFFER_SIZE server hangs?
      // TODO: Send a termination code? (eg. 200)
      while (chars_read == MSG_BUFFER_SIZE) {
        memset(buffer, 0, MSG_BUFFER_SIZE);
        chars_read = fread(buffer, sizeof(char), MSG_BUFFER_SIZE, file);
	// Check error.
	if (chars_read == 0) {
          if (ferror(file) != 0) {
            printf("Error reading file\n");
            return INT_ERR;
	  } else if (feof(file)) {
            printf("End of file.\n");
	    return INT_ERR;
	  }
	}
        // Send chars.
	printf("sending: %s \n", buffer); //DBG
        if (send(conn, buffer, chars_read, 0) < 0) {
          return CONN_ERR;
	}	
      }

      // Receive code.
      if (recv(conn, *code, 3, 0) < 0) {
        return CONN_ERR;
      }
      // Receive results.
      if (recv(conn, *response, MSG_BUFFER_SIZE, 0) < 0) {
        return CONN_ERR;
      }
      
      return OK;
    } else {
      return PROTO_ERR;
    }
  } else if (strcmp(tag, "UPLOAD") == 0) {
    // TODO: Implement "UPLOAD".
    return 0;
  } else {
    return COMM_ERR;
  }
}

// (Client side) Parse the arguments provided by command line, 
// into an sendable command (as per protocol).
// Return:
// OK on success,
// COMM_ERR when parsing and unsupported command,
// PARAM_ERR when parsing an invalid parameter.
int parse_command(char **argv, int i, char **command) {
  char *cmd = *command;
  memset(cmd, 0, MSG_BUFFER_SIZE);
  // Check '-'
  if (argv[i][0] != '-') {
    return PARAM_ERR;
  }
  // Parse command.
  switch(argv[i][1]) {
    case 'l':
      sprintf(cmd, "LIST");
      break;
    case 'e':
      sprintf(cmd, "EXEC");
      break;
    case 's':
      sprintf(cmd, "SIZE");
      break;
    case 'u':
      sprintf(cmd, "UPLOAD");
      break;
    case 'd':
      sprintf(cmd, "DOWNLOAD");
      break;
    default:
      return COMM_ERR;
  }

  // Append arguments.
  while(argv[++i]) {
    strcat(cmd, " ");
    strcat(cmd, argv[i]);
  }

  return OK;      
}

// (Server side) Parse command from msg, execute it, 
// place output in e_result and set e_code accordingly.
// Return:
// OK on success,
// COMM_ERR when parsing and unsupported command,
// PARAM_ERR when parsing an invalid parameter.
// INT_ERR when cannot popen. Launch perror() to print details.
int execute_command(char *msg, int conn, char **e_result, char **e_code) {
  // Command to be executed.
  char cmd[MSG_BUFFER_SIZE];
  memset(cmd, 0, MSG_BUFFER_SIZE);
  
  // Parse msg into cmd (system command).
  char *tag = strtok(msg, " ");
  if (strcmp(tag, "LIST") == 0) {
    // LIST implementation.
    strcat(cmd, LIST);  
    return exec(cmd, e_result, e_code);
  } else if (strcmp(tag, "EXEC") == 0) {
    // EXEC implementation.
    strcat(cmd, strtok(NULL, "\0"));
    return exec(cmd, e_result, e_code);
  } else if (strcmp(tag, "SIZE") == 0) {
    // SIZE implementation.
    char *filename = strtok(NULL, "\0");
    strcat(cmd, SIZE);
    strcat(cmd, filename);
    return exec(cmd, e_result, e_code);
  } else if (strcmp(tag, "DOWNLOAD") == 0) {
    // DOWNLOAD implementation.
    char *source = strtok(NULL, " ");
    char *dest = strtok(NULL, " ");

    // Open file for writing.
    FILE *file = fopen(dest, "w");
    // Check error.
    if (file == NULL) {
      // printf("Could not open file for writing: %s", dest); //DBG
      return INT_ERR;
    }

    // Send 200 (Ready).
    if (send(conn, "200", 3, 0) < 0) {
      return CONN_ERR;
    }

    // Read from connection and write to file.
    char buffer[MSG_BUFFER_SIZE];
    int recv_bytes = MSG_BUFFER_SIZE;
    while ( recv_bytes == MSG_BUFFER_SIZE) {
      // Receive chars.
      recv_bytes = recv(conn, buffer, MSG_BUFFER_SIZE, 0);
      // Check errors.
      if (recv_bytes < 0) {
        fclose(file);
        return CONN_ERR;
      } else if (recv_bytes == 0) {
        // Connection closed.
        fclose(file);
	return CONN_ERR;
      } else {
        // Write to file.
        fwrite(buffer, sizeof(char), recv_bytes, file);
      }
    }
    // Done
    fclose(file);
    sprintf(*e_code, "200");
    sprintf(*e_result, "File downloaded.%s", TERMINATION_STRING);

    return OK;
  } else if (strcmp(tag, "UPLOAD") == 0) {
  // TODO: implement UPLOAD.
  } else {
    // Command not supported.
    sprintf(*e_result, "Command not supported.");
    sprintf(*e_code, "400");

    return PROTO_ERR;
  }
}
