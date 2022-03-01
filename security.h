// Authentication functions.
//
#include "networking.h"
#define MSG_BUFFER_SIZE 256

// Hash function.
// Taken from: http://www.cse.yorku.ca/~oz/hash.html
uint64_t hash(unsigned char *str)
{
  // using int because long seems to be platform dependent 
  // TODO: Check use of long.
  uint64_t hash = 5381;
  int c;

  while (c = *str++) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

// Returns a unique token
uint64_t generateToken(char *passphrase) {
  return  hash(passphrase);
}

// Generate a pseudo-random 64 bits number.
uint64_t long_rand() {
  uint32_t upper = rand();
  uint32_t lower = rand();

  return ((uint64_t) upper << 32) | lower;  
}

// Perform client authentication, from server side.
// Return less than 0 on error.
int authenticate_client(int conn, uint64_t server_token) {
  char msg[MSG_BUFFER_SIZE], response[MSG_BUFFER_SIZE];
  int recv_bytes;
  memset(msg, 0, MSG_BUFFER_SIZE);
  memset(response, 0, MSG_BUFFER_SIZE);

  // Receive HELO
  if( (recv_bytes = recv(conn, response, MSG_BUFFER_SIZE, 0)) < 0) {
    return -1;	  
  }
  response[recv_bytes] = '\0';
  if( (strcmp(response, "HELO") != 0)) {
    return -2;
  }

  // Send Challenge
  send(conn, "300", 3, 0);
  uint64_t challenge = long_rand();
  uint64_t xor = server_token | challenge;

  printf("Sending xor: %lu\n", xor);
  if (send_long(conn, xor) < 0) {
    printf("Error sending long.\n");
  }
  
  return 1;
}

// Perform server authentication, from client side.
// Return less than 0 on error.
int authenticate_server(int conn, uint64_t server_token, uint64_t client_token) {
  char msg[MSG_BUFFER_SIZE], response[MSG_BUFFER_SIZE];
  int recv_bytes;
  memset(msg, 0, MSG_BUFFER_SIZE);
  memset(response, 0, MSG_BUFFER_SIZE);

  // Send HELO
  strcpy(msg, "HELO");
  send(conn, msg, strlen(msg), 0);
  if ((recv_bytes = recv(conn, response, MSG_BUFFER_SIZE, 0)) < 0 ) {
    return -1;	  
  }

  // Receive challenge
  response[recv_bytes] = '\0';
  if(strcmp(response, "300") != 0) {
  return -2;
  }
  uint64_t xor;
  uint64_t challenge;

  if(recv_long(conn, &xor) < 0) {
    printf("Error receiving long.\n");
  }
  printf("Received xor: %lu\n", xor);

  return 1;
}

