// Authentication server-client..
//
#include <inttypes.h>
#include "security.h"
#include "networking.h"
#include "constants.h"

#if defined _WIN32
  #define sleep(X) Sleep(X * 1000)
#endif

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
  // Wait if connection is slow.
  for(int it = 0; (it < 3) && 
      (recv_bytes = recv(conn, response, MSG_BUFFER_SIZE, 0)) == 0; it++) {
    sleep(1);		  
  }
  if(recv_bytes <= 0) {
    return -2;	  
  }
  response[recv_bytes] = '\0';

  char *auth = strtok(response, " ");
  char *_;
  uint64_t enc1 = strtoull(strtok(NULL, " "), &_, 0);
  uint64_t enc2 = strtoull(strtok(NULL, "\0"), &_, 0);
  //char *str_enc1 = strtok(NULL, " ");
  //char *str_enc2 = strtok(NULL, "\0");
  //uint64_t enc1 = strtoull(str_enc1, &_, 0);
  //uint64_t enc2 = strtoull(str_enc2, &_, 0);
  printf("%s %"PRIu64" %"PRIu64"\n", auth, enc1, enc2); //

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
  sprintf(msg, "AUTH %"PRIu64" %"PRIu64, enc1, enc2);
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
