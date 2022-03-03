// Authentication functions.
//
#include <time.h>
#include <string.h>
#include <inttypes.h>
#include "networking.h"
#include <stdlib.h>
#define MSG_BUFFER_SIZE 256

// Flush stdin
int flushstdin(void) { 
  int ch; 
  while (((ch = getchar()) != '\n') && (ch != EOF)) /* void */; 
  return ch == EOF ? EOF : 0; 
}

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

// Set rand() seed.
void set_rand_seed() {
  // Only run once.
  static int run = 1;
  if (run) {
    srand(time(0));
    run = 0;
  }
}

// Generate a pseudo-random 64 bits number.
uint64_t long_rand() {
  uint64_t random = 0;
  uint64_t shift = 8;
  uint64_t mask = 255;
  uint64_t temp;
  int size = sizeof(uint64_t) * 8;

  // Set seed. Randomize between executions.
#ifdef __linux__
  set_rand_seed();
#elif defined _WIN32
  srand(time(0));
#endif
  // Build up a 64 bits number with subsequent calls to rand().
  for(int i = 0; i < size; i += shift) {
    temp = (uint64_t) rand();
    temp = temp & mask;
    random = random << shift;
    random = random | temp;
  }

  return random;
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
  if(send(conn, "300", 3, 0) < 0) {
    return -1;
  }
  uint64_t challenge = long_rand();
  uint64_t xor = server_token ^ challenge;
  //printf("Challenge: %llu\n", challenge);
  //printf("Xor: %llu\n", xor);
  if (send_long(conn, xor) < 0) {
    return -1;
  }

  // Receive AUTH
  if((recv_bytes = recv(conn, response, MSG_BUFFER_SIZE, 0)) < 0) {
    return -1;		  
  }
  response[recv_bytes] = '\0';

  char *auth = strtok(response, " ");
  char *_;
  uint64_t enc1 = strtoull(strtok(NULL, " "), &_, 0);
  uint64_t enc2 = strtoull(strtok(NULL, " "), &_, 0);
  /*
  char *str_enc1 = strtok(NULL, " ");
  char *str_enc2 = strtok(NULL, " ");
  uint64_t enc1 = strtoull(str_enc1, &_, 0);
  uint64_t enc2 = strtoull(str_enc2, &_, 0);
  printf("%s %s\n", str_enc1, str_enc2);
  */
  //printf("%s %llu %llu\n", auth, enc1, enc2); //

  // Check challenge.
  if(strcmp(auth, "AUTH") == 0) {
    uint64_t client_token = enc1 ^ xor;
    uint64_t recv_challenge = enc2 ^ client_token;
    //printf("Received client token: %llu \n", client_token);
    //printf("Received challenge: %llu\n", challenge);
    if (challenge == recv_challenge) {
      // Authentication successful
      if (send(conn, "200", 4, 0) < 0) {
        return -1;
      }

      return 1; // Done.
    }
  }
  // Authentication failed
  if (send(conn, "400", 4, 0) < 0 ) {
    return -1;
  }

  return 0;
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
    return -1;
  }
  // printf("Received xor: %llu\n", xor);
  challenge = xor ^ server_token;
  //printf("Client token: %llu\n", client_token);
  //printf("Server token: %llu\n", server_token);
  //printf("Xor: %llu\n", xor);
  //printf("Challenge: %llu\n", challenge);

  // Send AUTH response
  uint64_t enc1;
  enc1 = xor ^ client_token;
  uint64_t enc2;
  enc2 = challenge ^ client_token;
  //printf("enc1: %llu enc2: %llu\n", enc1, enc2);
  sprintf(msg, "AUTH %llu %llu", enc1, enc2);
  //printf("Sending %s\n", msg); //
  if (send(conn, msg, strlen(msg), 0) < 0) {
    return -1;	  
  }

  // Final Receive
  if((recv_bytes = recv(conn, response, MSG_BUFFER_SIZE, 0)) < 0) {
    return -1;
  }
  response[recv_bytes] = '\0';
  int code = atoi(response);
  // printf("Auth: received %d.\n", code);
  if(code != 200) {
    return 0;
  } 

  return 1;
}

