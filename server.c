#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "security.h"
#include "queue.h"
#include "synchronization.h"
#include "threads.h"
#include "constants.h"
#include "commands.h"

#define DEFAULT_PORT 8888
#define DEFAULT_N_THREADS 10
#ifdef __linux__
  #define DEFAULT_LOG_FILE "/tmp/server.log"
#elif defined _WIN32
  #define DEFAULT_LOG_FILE "C:\\\\Windows\\Temp\\server.log"
#endif
#define CONF_ARRAY_SIZE 10

// Server token. Needed for authentication.
uint64_t server_token;
// Queue for incoming connections. Threads take connections from here.
struct Queue *connections;
// Critical section variable to make access to the connections queue exclusive..
void *connections_cs;

// Log message into the given file.
void log_to_file(char *message, char *filename) 
{
  FILE *file = fopen(filename, "a");

  fwrite(message, 1, strlen(message), file);
  fwrite("\n", 1, 1, file);

  fclose(file);
}

// Get command line parameters.
// Return 0 on success, -1 otherwise.
int get_parameters(
	int argc, char **argv, int *port, int *n_threads, 
	char **conf_file, int *s, char **log_file)
{
  char *option;
  int temp;

  for(int i = 1; i < argc; i++)
  {
    option = argv[i];


    if(option[0] == '-')
    {
      switch(option[1]) 
      {
        case 'p':
		temp = atoi(argv[i + 1]);
		if (temp == 0) 
		{ 
		  perror("Error: port not acceptable.\n"); 
		  return -1; 
		}
	       	else { *port = temp; }
		i++;
		break;
	case 'n':
		temp = atoi(argv[i + 1]);
		if (temp == 0) 
		{ 
		  perror("Error: number of threads not acceptable.\n"); 
		  return -1; 
		} 
		else { *n_threads = temp; }
		i++;
		break;
	case 'c':
		*conf_file = argv[i + 1];
		i++;
		break;
	case 's':
		*s = 1;
		break;
	case 'l':
		*log_file = argv[i + 1];
		i++;
		break;
	default:
		perror("Parameter not supported.\n");
		return -1;
      }
    }
    else { return -1; }
  }

  return 0;
}

// Read parameters from a configuration file.
// Sets argc and argv as if they were read from command line.
// Argv is dynamically allocated, as the pointer returned.
// Returns a pointer to char, where the parameter values,
// pointed by the the pointers in argv, are stored.
char* parse_conf_file(char *filename, int *argc, char ***argv)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL) 
  {
    perror("Error opening file.\n");
    return NULL;
  }

  char *line = NULL;
  size_t zero = 0;
  if (getline(&line, &zero, file) == -1) 
  {
    perror("Error reading file.");
    fclose(file);
    return NULL;
  }

  char **array = malloc(sizeof(char *) * CONF_ARRAY_SIZE);
  if (array == 0) 
  {
    perror("Error with malloc.\n");
    fclose(file);
    return NULL;
  }

  int c = 1;
  array[c] = strtok(line, " ");
  while((array[++c] = strtok(NULL, " "))&& c < CONF_ARRAY_SIZE);
 
  *argc = --c;
  *argv = array;
  fclose(file);

  return line;
}

void update_parameters(char *conf_file, int *p, int *n) 
{
  int argc;
  char **argv;
  char *line;

  line = parse_conf_file(conf_file, &argc, &argv);
  if(line == NULL) 
  {
    perror("Error parsing configuration file.\n");
  }
  if (get_parameters(argc, argv, p, n, NULL, NULL, NULL) != -1)
  {
    printf("Parameters updated. port: %d  threads: %d \n", *p, *n);
  }

  free(argv);
  free(line);
}

// Execution of a thread.
// Waits for a connection, performs authentication, 
// execute command and respond with results.
void thread_exec() {
  while(1) {
    // Wait for a connection.
    if(wait_semaphore() == -1) { 
      perror("Could not wait on semaphore.\n"); 
      continue;
    }

    // Get connection from queue.
    enter_cs(connections_cs);
    int conn = dequeue(connections);
    leave_cs(connections_cs);
    //printf("Handling connection: %d\n", conn);

    // Perform authentication.
    if (authenticate_client(conn, server_token) == 1) {
      printf("Authentication successful: %d.\n", conn);
    } else {
      printf("Authentication failed: %d.\n", conn);
      continue;
    }

    // Get commands.
    char msg[MSG_BUFFER_SIZE];
    memset(msg, 0, MSG_BUFFER_SIZE);
    int c = recv(conn, msg, MSG_BUFFER_SIZE, 0);

    if (c > 0) {
      // Execute command. 
      char *result = execute_command(msg, c);
      // Send result.
      if (send(conn, result, strlen(result), 0) < 0) {
        perror("Could not send results back.");	      
      }
      free(result);
      // Log request.
      // char *log_msg;
      // log_to_file(log_msg, log_file);
      printf("done with %d\n", conn); // print ip 
    } else if (c == 0) {  // Connection closed.	
      printf("Connection closed: %d\n", conn); // print client ip?
    } else {  // Error
      printf("Connection broken: %d\n", conn); // print client ip?
    }
  }
}


// Main.
int main (int argc, char **argv) 
{
  unsigned long int a;
  printf("Unsigned long int: %lu\n", sizeof(a));
  uint64_t b;
  printf("Uint64_t : %lu\n", sizeof(b));

  int port = DEFAULT_PORT;
  int n_threads = DEFAULT_N_THREADS;
  char *conf_file = NULL;
  int s = 0;
  char *log_file = DEFAULT_LOG_FILE;
  char passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters.
  if(get_parameters(argc, argv, &port, &n_threads, &conf_file, &s, &log_file) == -1)
  {
    return -1;
  }
  if(conf_file) { update_parameters(conf_file, &port, &n_threads); }
  printf("Settings set as: port: %d thread: %d log file: %s conf file: %s \n",
		  port, n_threads, log_file, conf_file);

  // Get passphrase.
  printf("Type passphrase: ");
  scanf("%s", passphrase);
  server_token = generateToken(passphrase);
  memset(passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  if(s)
  {
    printf("Server token: %"PRIu64" \n", server_token);
  }

  // Crea pool of threads.
   if (create_semaphore() == -1 ) { 
    perror("Could not create semaphore"); 
    return 1; 
  };
  create_thread_pool(n_threads, &thread_exec);
   
  // Setup socket.
  int sock = create_socket(port); 
  if(sock == -1) {
    perror("Could not create socket.");
    return -1;
  }
  struct sockaddr_in server, client;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = port;
  if( bind(sock ,(struct sockaddr *)&server, sizeof(server)) == -1 )
  {
    perror("Could not bind socket.");
    return -1;
  }
  if (listen(sock, n_threads) == -1) 
  {
    printf("Could not linsten on socket.");
    return -1;
  }
  printf("Listening...\n\n");

  // Accept connections
  connections = createQueue(n_threads);
  connections_cs = create_cs();
  while(1) {
    int client_sz = sizeof(struct sockaddr_in);
    int conn = accept(sock, (struct sockaddr *) &client, &client_sz);

    if(conn == -1) {
	    // SIGINT received?
      if (errno != EINTR) {
	      perror("Could not accept connection.");
	      break;
      }
      break;
    } else {
      printf("Received connection %d.\n", conn); // print client address?
      // Enqueue connection, in a critical section.
      enter_cs(connections_cs);
      while (enqueue(connections, conn) == -1) {
        #ifdef __linux__
        sleep(1);
        #elif defined _WIN32
        Sleep(1000);
        #endif
      }
      leave_cs(connections_cs);
      // Increment semaphore: unlocks a thread.
      increment_semaphore();
    }
  }

  // Terminate
  close_socket(sock);
  destroyQueue(connections);

  return 0;
}

