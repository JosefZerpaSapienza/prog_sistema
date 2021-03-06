#ifdef __linux__
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <unistd.h>
#elif defined _WIN32
  #include <stdint.h>
  #include <winsock2.h>
  #define close(X) closesocket(X)
#endif


// Check if the machine uses little endian.
// Return 1 if true.
int little_endian() {
  // The answer is 42
  static const int num = 42;
  char *p = (char *)&num;

  // Check the endianness
  if ((int) *p == num) {
    return 1;
  } else {
    return 0;	  
  }
}

// Convert value to network byte order
uint64_t htonll(uint64_t value) {
  if (little_endian() == 1) {
    // Little Endian: convert to big endian
    const uint32_t high_part = htonl((uint32_t)(value >> 32));
    const uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));
    
    return ((uint64_t)(low_part) << 32) | high_part;
  } else {
    return value;
  }
}

// Convert network byte order value to machine order long.
uint64_t nlltoh(uint64_t value) {
  if (little_endian() == 1) {
    // Little Endian
    const uint32_t high_part = ntohl((uint32_t)(value >> 32));
    const uint32_t low_part = ntohl((uint32_t)(value & 0xFFFFFFFFLL));

    return ((uint64_t)(low_part) << 32) | high_part;
  } else {
    return value;
  }
}

// Send a long int through a socket.
int send_long(int conn, uint64_t value) {
  uint64_t n_val = htonll(value);
  if (send(conn, (char *)&n_val, sizeof(uint64_t), 0) < 0) {
    return -1;	  
  }
  return 0;
}

// Receive a long int through a socket.
int recv_long(int conn, uint64_t *value) {
  uint64_t n_val;
  int recv_bytes = 0;
  int size = sizeof(uint64_t);
  while(recv_bytes < size){
    recv_bytes += recv(conn, ((char *)&n_val) + recv_bytes, size - recv_bytes, 0);
  }
  *value = nlltoh(n_val);
  return 0;
}
