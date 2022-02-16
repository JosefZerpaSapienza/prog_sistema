#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "security.h"
#include "networking.h"

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

int authenticate(int sock) {
  char msg[MSG_BUFFER_SIZE], response[MSG_BUFFER_SIZE];
  int r;
  memset(msg, 0, MSG_BUFFER_SIZE);
  memset(response, 0, MSG_BUFFER_SIZE);

  strcpy(msg, "HELO");
  send(sock, msg, strlen(msg), 0);
  if ((r = recv(sock, response, MSG_BUFFER_SIZE, 0)) < 0 ) {
    return -1;	  
  }
  response[r] = '\0';
  if(strcmp(response, "300") != 0) {
  return -2;
  }
  unsigned long *challenge;
  printf("%ld\n", sizeof(*challenge));
  /*
  if ((r = recv(sock, challenge, 1, 0)) < 0 ) {
    return -1;	  
  }
  */


  return 1;
}




int main(int argc, char **argv) 
{
  char *ip;
  int port;
  char cmd;
  char *args;
  char s_passphrase[PASSPHRASE_BUFFER_SIZE];
  char c_passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters
  if(argc < 6) 
  {
    printf(USAGE);
    return -1;
  }
  get_parameters(argc, argv, &ip, &port, &cmd, &args);

  // Perform authentication
  printf("Type server passphrase: ");
  scanf("%s", s_passphrase);
  printf("Type client passphrase: ");
  scanf("%s", c_passphrase);
  unsigned long s_token = generateToken(s_passphrase);
  memset(s_passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  unsigned long c_token = generateToken(c_passphrase);
  memset(c_passphrase, 0, PASSPHRASE_BUFFER_SIZE);


  // Connect
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
	  /*
  // Resolve the server address and port
  if( getaddrinfo(ip, port, &server, &result) != 0) {
    printf("Couldn't parse ip address.");
    return 1;
  } */
  if( (connect(sock, (struct sockaddr *) &server, sizeof(server))) < 0) {
    perror("Error trying to connect.");
    return -1;
  }

  authenticate(sock);
  //char *msg = "Hello!!";
  //send(sock, msg, strlen(msg), 0);

  close_socket(sock);

}

