// Define program wide constants here.
//

#define REDIRECTION_FILE_PATH "out.log"
#define ARGV_SIZE 10
#define CODE_SIZE 3
#define PASSPHRASE_BUFFER_SIZE 256
#define MSG_BUFFER_SIZE 2048
#define SMALL_BUFFER_SIZE 32
// Termination string to be appended to result messages.
#define TERMINATION_STRING "\r\n\r\n"

// Return value for 'OK/SUCCESS'.
#define OK 0
// Return value for 'Invalid parameters'.
#define PARAM_ERR -1
// Return value for 'Authentication failed'.
#define AUTH_FAIL -2
// Return value for 'Connection error'.
#define CONN_ERR -3
// Return value for 'Protocol error'.
#define PROTO_ERR -4
// Return value for 'Command not supported'.
#define COMM_ERR -5
// Return value for 'Generic internal error'.
#define INT_ERR -6

#ifdef _WIN32
  #define perror(X) { printf(X); printf("ErrorCode: %lu\n", GetLastError()); }
#endif
