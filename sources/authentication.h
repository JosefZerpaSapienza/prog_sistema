// Authentication server-client..
//
#include <inttypes.h>
#include "constants.h"
#include "networking.h"
#include "security.h"

#if defined _WIN32
  #define sleep(X) Sleep(X * 1000)
#endif

// Perform client authentication, from server side.
// Return  0 on success.
int authenticate_client(int conn, uint64_t server_token) {
  char response[MSG_BUFFER_SIZE];
  int recv_bytes;

  // Receive HELO.
  memset(response, 0, MSG_BUFFER_SIZE);
  if ( (recv_bytes = recv(conn, response, 4, 0)) < 0) {
    return CONN_ERR;	  
  }
  response[recv_bytes] = '\0';

  // Check HELO.
  if ( (strcmp(response, "HELO") != 0)) {
      printf("AUTH: HELO not received. \n"); //DBG
    return PROTO_ERR;
  }

  // Send response code.
  if (send(conn, "300", 3, 0) < 0) {
    return CONN_ERR;
  }

  // Calculate and send challenge.
  uint64_t challenge = long_rand();
  uint64_t xor = server_token ^ challenge;
  if (send_long(conn, xor) < 0) {
    return CONN_ERR;
  }

  // Receive AUTH
  memset(response, 0, MSG_BUFFER_SIZE);
  if ((recv_bytes = recv(conn, response, MSG_BUFFER_SIZE, 0)) < 0) {
    return CONN_ERR;	  
  }

  // Parse AUTH response.
  char *auth = strtok(response, " ");
  if (strtok == NULL) {
	printf("Null string token. \n");
	return PROTO_ERR;
  }
  char *enc1_string == strtok(NULL, " ");
  if (enc1_string == NULL) {
	printf("Null string token. \n");
	return PROTO_ERR;
  }
  char *enc2_string == strtok(NULL, "\0");
  if (enc2_string == NULL) {
	printf("Null string token. \n");
	return PROTO_ERR;
  }
  char *_;
  // Parsing should be fine since received number
  // was produced internally client side.
  uint64_t enc1 = strtoull(enc1_string, &_, 0);
  uint64_t enc2 = strtoull(enc2_string, &_, 0);

  // Check AUTH.
  if(strcmp(auth, "AUTH") != 0) {
    if (send(conn, "400", 3, 0) < 0 ) {
      return CONN_ERR;
    }
      printf("AUTH: AUTH not received.. \n"); //DBG
    return PROTO_ERR;
  }

  // Check challenge.
  uint64_t client_token = enc1 ^ xor;
  uint64_t recv_challenge = enc2 ^ client_token;
  if (challenge == recv_challenge) {
    // Authentication successful
    if (send(conn, "200", 3, 0) < 0) {
      return CONN_ERR;
    }

    return OK;
  } else {
    // Authentication failed
    if (send(conn, "400", 3, 0) < 0 ) {
      return CONN_ERR;
    }

    return AUTH_FAIL;
  }
}

// Perform server authentication, from client side.
// Return less than 0 on error.
int authenticate_server(int conn, uint64_t server_token, uint64_t client_token) {
  char msg[MSG_BUFFER_SIZE], response[MSG_BUFFER_SIZE];
  int recv_bytes;

  // Send HELO
  memset(msg, 0, MSG_BUFFER_SIZE);
  sprintf(msg, "HELO");
  if (send(conn, msg, strlen(msg), 0) < 0) {
	return CONN_ERR;
  }

  // Receive response code.
  memset(response, 0, MSG_BUFFER_SIZE);
  if (recv_bytes = recv(conn, response, 3, 0) < 0) {
    return CONN_ERR;
  }
    
  // Check response code.
  if(strcmp(response, "300") != 0) {
    return PROTO_ERR;
  }

  // Receive and calculate challenge.
  uint64_t xor;
  if(recv_long(conn, &xor) < 0) {
    return CONN_ERR;
  }
  uint64_t challenge = xor ^ server_token;

  // Send AUTH response.
  uint64_t enc1 = xor ^ client_token;
  uint64_t enc2 = challenge ^ client_token;
  memset(msg, 0, MSG_BUFFER_SIZE);
  sprintf(msg, "AUTH %"PRIu64" %"PRIu64, enc1, enc2);
  if (send(conn, msg, strlen(msg), 0) < 0) {
    return CONN_ERR;	  
  }

  // Receive and check auth result.
  memset(response, 0, MSG_BUFFER_SIZE);
  if((recv_bytes = recv(conn, response, 3, 0)) < 0) {
    return CONN_ERR;
  }
  int code = atoi(response);
  if(code == 200) {
    return OK;
  } 

  return AUTH_FAIL;
}

