#ifdef __linux__
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <unistd.h>
#elif defined _WIN32
  #include <winsock2.h>
  #pragma comment(lib,"ws2_32.lib")
#endif

// Open socket handling windows api setup.
int create_socket(int port) 
{
  // If windows start up WSA
  #ifdef _WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
    printf("Failed. Error Code : %d",WSAGetLastError());
    return 1;
  }
  #endif

  return  socket(AF_INET , SOCK_STREAM , 0 );
}


// Close socket handling windows api cleanup.
void close_socket(int sock) 
{
  #ifdef __linux__
    close(sock);	
  // If windows stop WSA and close socket with win api.
  #elif defined _WIN32
    closesocket(sock);
    WSACleanup();
  #endif
}

