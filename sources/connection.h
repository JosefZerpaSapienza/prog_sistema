// Handle connection data.
//
#ifdef __linux__
  #include <arpa/inet.h>
  #include <netinet/in.h>
#elif defined _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #define inet_ntop(X, Y, W, Z) InetNtopW(X, Y, W, Z)
#endif

#pragma comment(lib, "Ws2_32.lib")

struct Connection {
  int conn;
  struct sockaddr_in address;
};

// Dynamically allocate a new Connection.
// Return NULL on failure.
struct Connection *new_connection(int conn, struct sockaddr_in address) {
  struct Connection *new_co = malloc(sizeof(struct Connection));
  if (co == NULL) {
	  printf("Could not malloc. \n");
	  return NULL;
  }
  new_co->conn = conn;
  new_co->address = address;
  return new_co;
}

// Get connection descriptor for send/recv.
int get_conn(struct Connection *co) {
  return co->conn;
}

// Get ip address.
void get_ip(struct Connection *co, char *ip, int length) {
#ifdef __linux__
  inet_ntop(AF_INET, &(co->address.sin_addr), ip, sizeof(struct sockaddr_in));
#elif defined _WIN32
  WSAAddressToStringA((struct sockaddr *) &(co->address), 
      sizeof(struct sockaddr_in), NULL, ip, (unsigned long int *) &length);
  ip = strtok(ip, ":");
#endif
}

// Get port.
int get_port(struct Connection *co) {
  return ntohs(co->address.sin_port);
}
