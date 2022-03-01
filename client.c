#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "security.h"

#define USAGE "Usage: a.out -h <ip_addr> -p <port> <cmd> \n\n"

#define PASSPHRASE_BUFFER_SIZE 256
#define MSG_BUFFER_SIZE 256

// Return 0 on success, -1 otherwise.
int get_parameters(
        int argc, char **argv, char **ip, int *port, char *cmd, char **args)
{
  char *option;

  for(int i = 1; i < argc; i++)
  {
    option = argv[i];


    if(option[0] == '-')
    {
      switch(option[1])
      {
        case 'h':
		*ip = argv[i + 1];
                i++;
                break;
        case 'p':
                *port = atoi(argv[i + 1]);
                if (*port == 0)
                {
                  perror("Error: port not acceptable.\n");
                  return -1;
                }
                i++;
                break;
        case 'l':
                *cmd = option[1];
		*args = argv[i + 1];
                break;
      }
    } else { return -1; }
  }

  return 0;
}


int main(int argc, char **argv) 
{
  char *ip;
  int port;
  char cmd;
  char *args;
  char server_passphrase[PASSPHRASE_BUFFER_SIZE];
  char client_passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters
  if(argc < 6) 
  {
    printf(USAGE);
    return -1;
  }
  get_parameters(argc, argv, &ip, &port, &cmd, &args);

  // Get tokens
  printf("Type server passphrase: ");
  scanf("%s", server_passphrase);
  printf("Type client passphrase: ");
  scanf("%s", client_passphrase);
  uint64_t server_token = generateToken(server_passphrase);
  memset(server_passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  uint64_t client_token = generateToken(client_passphrase);
  memset(client_passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  
  // Connect to server
  int sock = create_socket(port);
  if(sock == -1) {
    perror("Could not create socket.");
    return -1;
  }
  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = port;
  server.sin_addr.s_addr = inet_addr(ip);
  if( (connect(sock, (struct sockaddr *) &server, sizeof(server))) < 0) {
    perror("Error trying to connect.");
    return -1;
  }
  printf("\nConnection established.\n");

  // Perform authentication
  if(authenticate_server(sock, server_token, client_token) > 0) {
    printf("Authentication successful.\n");
  } else {
    printf("Authentication failed!\n");
  }
  //char *msg = "Hello!!";
  //send(sock, msg, strlen(msg), 0);

  // Done
  close_socket(sock);

}

