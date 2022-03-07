#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "security.h"
#include "constants.h"

#define USAGE "Usage: a.out -h <ip_addr> -p <port> <cmd> \n\n"

// Builds up the arguments part of the command supplied 
// by command line.
// Returns a dynamically allocated string.
char *parse_command(char **argv, int i) {
  // Allocate space.
  char *cmd = malloc(MSG_BUFFER_SIZE);
  memset(cmd, 0, MSG_BUFFER_SIZE);
  // Check '-'
  if (argv[i][0] != '-') {
    return NULL;
  }
  printf("ok2");
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
  printf("ok3");
  // Append arguments.
  while(argv[++i]) {
    strcat(cmd, " ");
    strcat(cmd, argv[i]);
  }

  return cmd;
}

// Parse parameters from argv.
// Return 0 on success, -1 otherwise.
int get_parameters(
        int argc, char **argv, char **ip, int *port, char **cmd)
{
  char *option;

  for(int i = 1; i < argc; i++)
  {
    option = argv[i];

    if(option[0] != '-'){
      return -1;
    }
    switch(option[1]) {
      case 'h':
        *ip = argv[i + 1];
        i++;
        break;
      case 'p':
        *port = atoi(argv[i + 1]);
         if (*port == 0) {
           perror("Error: port not acceptable.\n");
           return -1;
         }
         i++;
         break;
      default:
  printf("ok1");
        *cmd = parse_command(argv, i);
	if (*cmd == NULL) {
	  return -1;	  
	}
	return 0;
    }
  }
}


int main(int argc, char **argv) 
{
  char *ip;
  int port;
  char *cmd;
  char server_passphrase[PASSPHRASE_BUFFER_SIZE];
  char client_passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters.
  if(argc < 6) 
  {
    printf(USAGE);
    return -1;
  }
  get_parameters(argc, argv, &ip, &port, &cmd);

  // Get passphrases. 
  printf("Type server passphrase: ");
  scanf("%s", server_passphrase);
  //server_passphrase = (char *)"hello";
  printf("Type client passphrase: ");
  scanf("%s", client_passphrase);
  //client_passphrase = (char *)"world";
  uint64_t server_token = generateToken(server_passphrase);
  memset(server_passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  uint64_t client_token = generateToken(client_passphrase);
  memset(client_passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  // printf("server token: %llu\n", server_token);

  // Connect to server.
  int sock = create_socket(port);
  if(sock == -1) {
    perror("Could not create socket.");
    return -2;
  }
  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = port;
  server.sin_addr.s_addr = inet_addr(ip);
  if( (connect(sock, (struct sockaddr *) &server, sizeof(server))) < 0) {
    perror("Error trying to connect.");
    return -2;
  }
  printf("\nConnection established.\n");

  // Perform authentication.
  if(authenticate_server(sock, server_token, client_token) > 0) {
    printf("Authentication successful.\n");
  } else {
    printf("Authentication failed!\n");
    return -3;
  }

  // Send command.
  //char *msg = "Hello from the client!";
  if (send(sock, cmd, strlen(cmd), 0) < 0) {
    perror("Could not send command.");
    return -2;
  }

  // Receive result.
  char results[MSG_BUFFER_SIZE];
  int recv_bytes;
  if((recv_bytes = recv(sock, results, MSG_BUFFER_SIZE, 0) ) < 0) {
    perror("Could not receive results from server.");
    return -2;
  } else {
    results[recv_bytes] = '\0';
    printf("%s\n", results);
  }

  // Done
  close_socket(sock);
}

