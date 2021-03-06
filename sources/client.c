#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "authentication.h"
#include "commands.h"
#include "constants.h"

#define USAGE "\
\n \
Usage: ./client -h <ip_addr> -p <port> <cmd> \n \n \
Accepted commands are: \n \n \
-l <dir_path>        LIST: List contents of a directory. \n \
-s [file_path]       SIZE: Print file size. \n \
-u [source_file] [dest_file]    UPLOAD: Upload file from server location to client location. \n \
-d [source_file] [dest_file]     DOWNLOAD: Download file from client location to server location. \n \
-e [command]         EXEC: Execute command. \n \n \
"

// Parse parameters from argv.
// Return: 
// OK On success, 
// PARAM_ERR when parsing an invalid parameter,
// PROTO_ERR when missing command.
int get_parameters(
        int argc, char **argv, char **ip, int *port, char *cmd)
{
  // Read argv[].
  for(int i = 1; i < argc; i++)
  {
    char *option = argv[i];

    // Check '-'.
    if(option[0] != '-') {
      return PROTO_ERR;
    }
    // Parse option.
    switch(option[1]) {
      case 'h':
        *ip = argv[i + 1];
        i++;
	break;
      case 'p':
        *port = atoi(argv[i + 1]);
         if (*port == 0) {
           perror("Error: port not acceptable.\n");
           return PARAM_ERR;
         }
         i++;
         break;
      default:
         return parse_command(argv, i, cmd);
    }
  }
  // Missing command.
  return PROTO_ERR;
}

int main(int argc, char **argv) 
{
  char *ip;
  int port;
  char cmd[MSG_BUFFER_SIZE];
  char server_passphrase[PASSPHRASE_BUFFER_SIZE];
  char client_passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get command line parameters.
  int params = get_parameters(argc, argv, &ip, &port, (char *)&cmd);

  // Check errors.
  switch (params) {
    case PARAM_ERR:
      printf("Invalid parameter received. \n");
      printf("%s", USAGE);
      return PARAM_ERR;
    case PROTO_ERR:
      printf("Invalid input. \n");
      printf("%s", USAGE);
      return PROTO_ERR;
  }

  // Get passphrases. 
  printf("Type server passphrase: ");
  scanf("%s", server_passphrase);
  printf("Type client passphrase: ");
  scanf("%s", client_passphrase);

  // Generate tokens.
  uint64_t server_token = generateToken(server_passphrase);
  memset(server_passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  uint64_t client_token = generateToken(client_passphrase);
  memset(client_passphrase, 0, PASSPHRASE_BUFFER_SIZE);

  // If windows start up WSA
#ifdef _WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
    perror("WSA startup failed");
    return INT_ERR;
  }
#endif

  // Connect to server.
  int conn = socket(AF_INET , SOCK_STREAM , 0 );
  if(conn == -1) {
    perror("Could not create socket.");
    return CONN_ERR;
  }
  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = inet_addr(ip);
  if( (connect(conn, (struct sockaddr *) &server, sizeof(server))) < 0) {
    perror("Error trying to connect.");
    return CONN_ERR;
  }
  printf("\nConnection established.\n");

  // Perform authentication.
  int auth = authenticate_server(conn, server_token, client_token);
  // Check errors.
  switch (auth) {
    case AUTH_FAIL:
      printf("Authentication failed!\n");
      close(conn);
      return AUTH_FAIL;
    case CONN_ERR:
      printf("Authentication error: Connection error.\n");
      close(conn);
      return CONN_ERR;
    case PROTO_ERR:
      printf("Authentication error: wrong protocol.\n");
      close(conn);
      return PROTO_ERR;
  }
  printf("Authentication successful.\n\n");

  // Send command.
  if (send(conn, cmd, strlen(cmd), 0) < 0) {
    perror("Could not send command.");
    close(conn);
    return CONN_ERR;
  }

  // Handle response.
  char results[MSG_BUFFER_SIZE];
  char code[CODE_SIZE + 1];
  memset(results, 0, sizeof(results));
  memset(code, 0, sizeof(code));
  int ret = handle_response((char *)&cmd, conn, (char *)&results, (char *)&code);
  // Check errors.
  switch (ret) {
    case OK:
      break;
    case CONN_ERR:
      perror("Could not receive response code from server.");
      break;
    case PROTO_ERR:
      printf("Server returned bad code: %s \n", code);
      printf("%s", results);
      break;
    case INT_ERR:
      perror("Internal error.");
      break;
    case COMM_ERR:
      printf("Command not supported. \n");
      break;
    default:
      printf("Unexpected return value!\n");
      close(conn);
      return INT_ERR;
  }

  // Print results.
  if (results != NULL) {
    printf("%s", results);
  }

  // Done
  close(conn);
  // If windows stop WSA and close socket with win api.
#ifdef _WIN32
  WSACleanup();
#endif

  return ret;
}

