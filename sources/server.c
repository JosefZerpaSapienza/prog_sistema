#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "authentication.h"
#include "commands.h"
#include "connection.h"
#include "constants.h"
//#include "deamonize.h"
#include "logging.h"
#include "threads.h"

#define CONF_ARRAY_SIZE 10
#define DEFAULT_PORT 8888
#define DEFAULT_N_THREADS 10
#ifdef __linux__
  #define DEFAULT_LOG_FILE "/tmp/server.log"
#elif defined _WIN32
  #define sleep(X) Sleep(X * 1000)
  #define DEFAULT_LOG_FILE "C:\\\\Windows\\Temp\\server.log"
#endif

// Connection port.
int port = DEFAULT_PORT;
// Configuration file path.
char *conf_file = NULL;
// Number of threads to launch.
int n_threads = DEFAULT_N_THREADS;
// Server token. Needed for authentication.
uint64_t server_token;
// Queue for incoming connections. Threads take connections from here.
struct Queue *connections;
// Critical section variable to make access to the connections queue exclusive.
void *connections_cs;
// Sighup flag.
int sighup = 0;

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

  return OK;
}

// Read parameters from a configuration file.
// Sets argc and argv as if they were read from command line.
// Argv is dynamically allocated, as the pointer returned.
// Returns a pointer to char, where the parameter values,
// pointed by the the pointers in argv, are stored.
char* parse_conf_file(char *filename, int *argc, char ***argv)
{
  if (filename == NULL) {
    printf("No configuration file defined. \n");
    return NULL;
  }

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

  if (conf_file == NULL) { return; }

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
  // Print thread ids.
  //printf("t_id: %d \n", get_thread_id());

  while(1) {
    // Wait for a connection.
    if(wait_semaphore() == -1) { 
      perror("Could not wait on semaphore.\n"); 
      continue;
    }

    // Get connection from queue.
    enter_cs(connections_cs);
    struct Connection *co = (struct Connection *) dequeue(connections);
    leave_cs(connections_cs);
    
    // Check dequeue. Shouldn't be necessary: just in case.
    if (co == NULL) {
      printf("Started thread didn't find a connection.\n");
      continue;
    }

    // Get connection info.
    int conn = get_conn(co);
    char ip[SMALL_BUFFER_SIZE];
    get_ip(co, ip, SMALL_BUFFER_SIZE);
    int port = get_port(co);

    // Perform authentication.
    int auth = authenticate_client(conn, server_token);
    // Check errors
    switch (auth) {
      case AUTH_FAIL:
        printf("Authentication failed - challenge failed: %s.\n", ip);
        break;
      case CONN_ERR:
        printf("Authentication failed - connection error: %s.\n", ip);
        break;
      case PROTO_ERR:
        printf("Authentication failed - wrong protocol: %s.\n", ip);
        break;
    }
    if(auth < 0) {
      free(co);
      continue;
    }

    // Receive command.
    char msg[MSG_BUFFER_SIZE];
    char *code = malloc(4);
    char *result = malloc(MSG_BUFFER_SIZE);
    memset(msg, 0, MSG_BUFFER_SIZE);
    memset(result, 0, MSG_BUFFER_SIZE);
    int recv_bytes = recv(conn, msg, MSG_BUFFER_SIZE, 0);

    // Check recv errors.
    if (recv_bytes > 0) {
      // Execute command. 
      int exec = execute_command(msg, conn, &result, &code);
      // Check execute errors.
      switch (exec) {
        case OK:
	  break;
	case CONN_ERR:
	  perror("Command execution: Connection error.\n"); 
	  break;
        case INT_ERR:
	  perror("Command Execution: Internal Error. ");
	  break;
	case PROTO_ERR:
	  printf("Command Execution: Command not supported. \n"); 
	  break;
	case COMM_ERR:
	  printf("Command execution: Returned with error. \n"); 
	  break;
	case PARAM_ERR:
	  printf("Command execution: Invalid parameter. \n"); 
	  break;
	default:
	  printf("Unexpected return value! %d \n", exec);
	  break;
      }

      // Send code.
      if(send(conn, code, strlen(code), 0) < 0) {
        printf("Could not send code back: %s.\n", ip);
	perror("a");
	break;
      }

      // Send result.
      if (send(conn, result, strlen(result), 0) < 0) {
        printf("Could not send results back: %s.\n", ip);	      
	break;
      }

      // Log request.
      char *tag = strtok(msg, " ");
      log_request(get_thread_id(), ip, port, tag);

      //  printf("done with %s\n", ip); //DBG

    } else if (recv_bytes == 0) { 
      // Connection closed.	
      printf("Connection closed: %s\n", ip);
    } else {  
      // Error
      printf("Connection broken: %s\n", ip);
    }

    // Clean.
    free(co);
    free(code);	
    free(result);
  }
}

// Handler function for SIGHUP.
void sighup_handler(int signo) {
  printf("SIGHUP Received.\n");

  // If no conf file defined: do nothing.
  if (conf_file == NULL) {
    return; 
  }

  // Set sighup flag, making main repeat the sighup loop.
  // And update parameters.
  sighup = 1;
  update_parameters(conf_file, &port, &n_threads);
}

// Main.
int main (int argc, char **argv) 
{
  int s = 0;
  char *log_file = DEFAULT_LOG_FILE;
  char passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters.
  if(get_parameters(argc, argv, &port, &n_threads, &conf_file, &s, &log_file) == -1) {
    return -1;
  }
  if(conf_file) { update_parameters(conf_file, &port, &n_threads); }
  set_log_file(log_file);
  printf("Settings: \nport: %d threads: %d log_file: %s conf_file: %s \n",
	port, n_threads, log_file, conf_file);

  // Get passphrase.
  printf("Type passphrase: ");
  scanf("%s", passphrase);
  server_token = generateToken(passphrase);
  memset(passphrase, 0, PASSPHRASE_BUFFER_SIZE);
  if(s) {
    printf("Server token: %"PRIu64" \n", server_token);
  }

#ifdef __linux__
  //deamonize();
#endif

#ifdef __linux__
  // Set up SIGHUP handler.
  struct sigaction action;
  action.sa_handler = sighup_handler;
  if (sigaction(SIGHUP, &action, NULL) != 0) {
    perror("Could not set signal handler. \n");
  }
#endif

  // SIGHUP loop.
  while(1) {
    // Reset sighup.
    sighup = 0;

    // Crea pool of threads.
    if (create_semaphore() == -1 ) { 
      perror("Could not create semaphore"); 
      return 1; 
    };
    create_thread_pool(n_threads, &thread_exec);
   
    // Setup listening socket.
    int sock = create_socket(port); 
#ifdef __linux__
    int true = 1;
    // Set socket as reusable to avoid reconnection errors.
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));
#endif
    if(sock == -1) {
      perror("Could not create socket.");
      return -1;
    }
    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if( bind(sock ,(struct sockaddr *)&server, sizeof(server)) == -1 ) {
      perror("Could not bind socket.");
      return -1;
    }
    if (listen(sock, n_threads) == -1) {
      printf("Could not linsten on socket.");
      return -1;
    }
    printf("Listening...\n\n");

    // Accept connections.
    connections = createQueue(n_threads);
    connections_cs = create_cs();
    while(sighup != 1) {
      int client_sz = sizeof(struct sockaddr_in);
      int conn = accept(sock, (struct sockaddr *) &client, &client_sz);

      if(conn == -1) {
        // SIGINT received?
        if (errno != EINTR) {
          printf("Could not accept connection.\n");
          break;
        }
        break;
      } else {
        struct Connection *co = new_connection(conn, client);
        // Enqueue connection, in a critical section.
        enter_cs(connections_cs);
        while (enqueue(connections, (void *) co) == -1) {
          sleep(1);
        }
        leave_cs(connections_cs);
        // Increment semaphore: unlocks a thread.
        increment_semaphore();
      }
    }
  
    // Wait for pending connections to be fulfilled.
    while(isEmpty(connections) == 0) {
      printf("Waiting on pending connections.\n");
      sleep(1);
    }

    // Close and clean. 
    close_socket(sock);
    destroyQueue(connections);
  }

  return 0;
}

