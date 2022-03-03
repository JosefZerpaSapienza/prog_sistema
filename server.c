#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "security.h"
#include "queue.h"
#include "synchronization.h"
#include "threads.h"

#define DEFAULT_PORT 8888
#define DEFAULT_N_THREADS 10
#ifdef __linux__
  #define DEFAULT_LOG_FILE "/tmp/server.log"
#elif defined _WIN32
  #define DEFAULT_LOG_FILE "C:\\\\Windows\\Temp\\server.log"
#endif
#define PASSPHRASE_BUFFER_SIZE 256
#define CONF_ARRAY_SIZE 10
#define MSG_BUFFER_SIZE 256

uint64_t server_token;
// Queue for incoming connections.
struct Queue *connections;
// Object for handling critical section to access connections queue.
void *connections_cs;

//
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

void handle_message(char *msg, int len) {
  printf("Received %s. \n", msg);
}

void thread_exec() {
  while(1) {
    // Wait for a connection
    if(wait_semaphore() == -1) { perror("Could not wait on semaphore.\n"); }

    // Get connection from queue
    enter_cs(connections_cs);
    int conn = dequeue(connections);
    leave_cs(connections_cs);
    //printf("handling %d\n", conn);

    // Perform authentication
    if (authenticate_client(conn, server_token) == 1) {
      printf("Authentication successful: %d.\n", conn);
    } else {
      printf("Authentication failed: %d.\n", conn);
    }

    // Handle connection
    int established = 1;
    while(established) {
      // Get message.
      char msg[MSG_BUFFER_SIZE];
      memset(msg, 0, MSG_BUFFER_SIZE);
      int c = recv(conn, msg, MSG_BUFFER_SIZE, 0);

      if (c > 0) {
        handle_message(msg, c);
      } else if (c == 0) {  // Connection closed.	
        established = 0;
	printf("Connection closed\n");
      } else {  // Error
        established = 0;
      }
    }
  }
}



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

  // Get parameters and check errors.
  if(get_parameters(argc, argv, &port, &n_threads, &conf_file, &s, &log_file) == -1)
  {
    return -1;
  }

  // Read parameters.
  if(conf_file) { update_parameters(conf_file, &port, &n_threads); }
  
  //Print settings.
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
   
  // Attendi connessioni.
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
  connections = createQueue(n_threads);
  connections_cs = create_cs();

  // Accept connections
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
      printf("Received connection %d.\n", conn);
      // Enqueue connection in a critical section.
      enter_cs(connections_cs);
      while (enqueue(connections, conn) == -1) {
        #ifdef __linux__
        sleep(1);
        #elif defined _WIN32
        Sleep(1000);
        #endif
      }
      leave_cs(connections_cs);
      // Increment semaphore.
      increment_semaphore();
    }
  }

  close_socket(sock);

  return 0;
}

