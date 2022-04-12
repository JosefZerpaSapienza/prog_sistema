#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "authentication.h"
#include "commands.h"
#include "connection.h"
#include "constants.h"
#ifdef __linux__
#include "daemonize.h"
#endif
#include "logging.h"
#include "threads.h"

#define USAGE "\
\n \
Usage: ./server <options> \n \n \
Accepted options are: \n \n \
-s                 Print passphrase token. \n \
-p [port]          Set port number. \n \
-n [n_threads]     Set number of threads. \n \
-c [path]          Set configuration file path. \n \
-l [path]          Set log file path. \n \n \
"

#define DEFAULT_PORT 8888
#define DEFAULT_N_THREADS 10
#ifdef __linux__
  #define DEFAULT_LOG_FILE "/tmp/server.log"
#elif defined _WIN32
  #define sleep(X) Sleep(X * 1000)
  #define DEFAULT_LOG_FILE "C:\\\\Windows\\Temp\\server.log"
#endif

// Configuration variables.
//
// Connection port.
int port = DEFAULT_PORT;
// Number of threads to launch.
int n_threads = DEFAULT_N_THREADS;
// Configuration file path.
char *conf_file = NULL;
// Log file path.
char *log_file = DEFAULT_LOG_FILE;
// Print option (-s):
int s = 0;

// Server token. Needed for authentication.
uint64_t server_token;
// Queue for incoming connections. Threads take connections from here.
struct Queue *connections;
// Critical section variable to make access to the connections queue exclusive.
void *connections_cs;
// Sighup flag.
int sighup = 0;

// Set configuration variables as defined by argv[].
// Conf variables aren't changed if parsing fails.
// Return OK on success,
// PARAM_ERR on failure.
int get_parameters(int argc, char **argv) {
  // Use temporary variables to store parsed configurations.
  // Initialize them as the actual current configuration values.
  int tport = port;
  int tn_threads = n_threads;
  int ts = s;
  char *tconf_file = conf_file;
  char *tlog_file = log_file;

  char *option;

  for(int i = 1; i < argc; i++) {
    option = argv[i];

    if(option[0] == '-') {
      switch(option[1]) {
        case 'p':
			tport = atoi(argv[i + 1]);
			if (tport == 0) {
			  perror("Error: port not acceptable.");
			  return PARAM_ERR;
			}
			i++;
			break;
		case 'n':
			tn_threads = atoi(argv[i + 1]);
			if (tn_threads == 0) {
			  perror("Error: number of threads not acceptable.");
			  return PARAM_ERR;
			}
			i++;
			break;
		case 's':
			ts = 1;
			break;
		case 'c':
			tconf_file = argv[i + 1];
			i++;
			break;
		case 'l':
			tlog_file = argv[i + 1];
			i++;
			break;
		default:
			printf("Parameter not supported.\n");
			return PARAM_ERR;
      }
    }
    else {
      return PARAM_ERR;
    }
  }

  // Copy temp variables into configuration variables.
  // (This makes sure all of the updates are performed only if the
  //  parsing reaches the end correctly.)
  port = tport;
  n_threads = tn_threads;
  s = ts;
  conf_file = tconf_file;
  log_file = tlog_file;

  return OK;
}

// Read parameters from a configuration file.
// Sets argc and argv as if they were read from command line.
// Return OK on success,
// INT_ERR on failure.
int parse_conf_file(char *fname, int *argc, char **argv) {
  // Check filename.
  if (fname == NULL) {
    printf("No configuration file defined. \n");
    return INT_ERR;
  }

  // Open file.
  FILE *file = fopen(fname, "r");
  if (file == NULL) {
    perror("Error opening file.\n");
    return INT_ERR;
  }

  // Read file.
  char *line = NULL;
  size_t zero = 0;
  if (getline(&line, &zero, file) == -1) {
    perror("Error reading file.");
    fclose(file);
    return INT_ERR;
  }

  // Populate argv[].
  int c = 1;
  // TODO: Check strtok return.
  argv[c] = strtok(line, " ");
  while((argv[++c] = strtok(NULL, " ")) && c < ARGV_SIZE);
 
  // Set argc.
  *argc = --c;

  fclose(file);

  return OK;
}

// Update parameters from a conf file, if it is set.
// Return OK on success,
// INT_ERR on failure.
int update_parameters() {
  // Check if conf file is set.
  if (conf_file == NULL) { return OK; }

  // Set up argc and argv.
  int argc;
  char *argv[ARGV_SIZE];

  // Parse conf file: populate argv and argc.
  if (parse_conf_file(conf_file, &argc, (char **)&argv) != OK) {
    perror("Error parsing configuration file.\n");
    return INT_ERR;
  }
  if (get_parameters(argc, argv) != OK) {
	printf("Could not update parameters. \n");
  }
  printf("Parameters updated. port: %d  threads: %d \n", port, n_threads);

  return OK;
}

// Send error to client.
// To be executed when an unexpected error occurs.
void send_error(int conn) {
  char *error_msg = "500 Server error.";
  if(send(conn, error_msg, sizeof(error_msg), 0) < 0) {
	printf("Network error.\n");
  }
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
    if (enter_cs(connections_cs) != OK) {
      printf("Could not enter Critical Section. \n");
      continue;
    }
    struct Connection *co = (struct Connection *) dequeue(connections);
    if (co == NULL) {
      printf("Could not dequeue. \n");
      continue;
    }
    if (leave_cs(connections_cs) != OK) {
      printf("Could not leave Critical Section.\n");
      send_error(get_conn(co));
      close(get_conn(co));
      free(co);
      continue;
    }


    // Get connection info.
    int conn = get_conn(co);
    char ip[SMALL_BUFFER_SIZE];
    if (get_ip(co, ip, SMALL_BUFFER_SIZE) != OK) {
      sprintf(ip, "PARSING_ERROR");
    }
    int cport = get_port(co);

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
      close(conn);
      free(co);
      continue;
    }

    char msg[MSG_BUFFER_SIZE];
    char code[CODE_SIZE + 1];
    char result[MSG_BUFFER_SIZE];

    // Receive command.
    memset(msg, 0, MSG_BUFFER_SIZE);
    int recv_bytes = recv(conn, msg, MSG_BUFFER_SIZE, 0);

    // Check recv errors.
    if (recv_bytes > 0) {
      // Execute command. 
      memset(result, 0, MSG_BUFFER_SIZE);
      int exec = execute_command(msg, conn, (char *)&result, (char *)&code);
      // Check execute errors.
      switch (exec) {
        case OK:
	  break;
	  case CONN_ERR:
        perror("Command execution: Connection error.");
	    break;
      case INT_ERR:
	    perror("Command Execution: Internal Error. ");
	    break;
	  case PROTO_ERR:
	    printf("Command Execution: Protocol error. \n");
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
	    break;
      }

      // Send result.
      if (send(conn, result, strlen(result), 0) < 0) {
        printf("Could not send results back: %s.\n", ip);	      
	    break;
      }

      // Log request.
      char *tag = strtok(msg, " ");
      if (log_request(get_thread_id(), ip, cport, tag) != OK) {
    	printf("Could not log. \n");
      }

      // printf("done with %s\n", ip); //DBG

    } else if (recv_bytes == 0) { 
      // Connection closed.	
      printf("Connection closed: %s\n", ip);
    } else {  
      // Error
      printf("Connection broken: %s\n", ip);
    }

    // Clean.
    free(co);
    close(conn);
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
  if (update_parameters() != OK) {
	printf("Could not update parameters. \n");
  }

  // Printf configurations.
  printf("Settings: \nport: %d threads: %d log_file: %s conf_file: %s \n",
	port, n_threads, log_file, conf_file);
}

// Main.
int main (int argc, char **argv) {
  char passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters.
  if(get_parameters(argc, argv) != OK) {
    printf("%s \n", USAGE);
	return PARAM_ERR;
  }

  // Read conf file.
  if(conf_file) {
	if (update_parameters() != OK) {
      printf("Could not update parameters. \n");
    }
  }

  // Set log file.
  set_logging(log_file);

  // Printf configurations.
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
  // Set up SIGHUP handler.
  struct sigaction action;
  action.sa_handler = sighup_handler;
  if (sigaction(SIGHUP, &action, NULL) != 0) {
    perror("Could not set signal handler. Configuration update at run-time is disabled.\n");
  }
#endif

#ifdef __linux__
  if (daemonize() != OK) {
    perror("Could not daemonize.");
  }
#endif

  // SIGHUP loop.
  while(1) {
    // Reset sighup.
    sighup = 0;

    // Create connection semaphore
    if (create_semaphore() == -1 ) { 
      perror("Could not create semaphore"); 
      return 1; 
    }
    // Crea pool of threads.
    if (create_thread_pool(n_threads, &thread_exec) != OK) {
      printf("Could not create thread pool. \n");
    }
   
  // If windows start up WSA
#ifdef _WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
    perror("WSA startup failed");
    return INT_ERR;
  }
#endif

    // Setup listening socket.
    int sock = socket(AF_INET , SOCK_STREAM , 0 );
    if (sock == -1) {
      perror("Could not create socket.");
      return INT_ERR;
    }

#ifdef __linux__
    int true = 1;
    // Set socket as reusable to avoid reconnection errors.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1) {
    	perror("Could not set socket as reusable. \
    			Updating settings at runtime might break connection.");
    }
#endif

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if( bind(sock ,(struct sockaddr *)&server, sizeof(server)) == -1 ) {
      perror("Could not bind socket.");
      return INT_ERR;
    }
    if (listen(sock, n_threads) == -1) {
      printf("Could not listen on socket.");
      return INT_ERR;
    }
    printf("Listening...\n\n");

    // Create connections queue.
    connections = createQueue(n_threads);
    if (connections == NULL) {
      printf("Could not create connections queue. \n");
      return INT_ERR;
    }

    //Create connections critical section.
    connections_cs = create_cs();
    if (connections_cs == NULL) {
      printf("Could not create connections critical section. \n");
      return INT_ERR;
    }

    // Accept connections.
    while(sighup != 1) {
      int client_sz = sizeof(struct sockaddr);
      int conn = accept(sock, (struct sockaddr *) &client, (socklen_t *)&client_sz);

      if(conn == -1) {
        // SIGINT received?
        if (errno != EINTR) {
          perror("Could not accept connection.\n");
          continue;
        }
        break;
      } else {
        struct Connection *co = new_connection(conn, client);
        if (co == NULL) {
          printf("Could not create connection structure. \n");
          send_error(conn);
          close(conn);
          continue;
        }
        // Enqueue connection, in a critical section.
        if (enter_cs(connections_cs) != OK) {
            printf("Could not enter critical section correctly. \n");
            send_error(conn);
            close(conn);
            continue;
        }
        // Wait if could not enqueue.
        while (enqueue(connections, (void *) co) == -1) {
          printf("Connection queue seems to be full. Waiting . . .\n");
          sleep(1);
        }
        if (leave_cs(connections_cs) != OK) {
          printf("Could not leave critical section correctly. \n");
          send_error(conn);
          close(conn);
          continue;
        }
        // Increment semaphore: unlocks a thread.
        if (increment_semaphore() == -1) {
          printf("Could not increment semaphore. \n");
          send_error(conn);
          close(conn);
          continue;
        }
      }
    }
  
    // Wait for pending connections to be fulfilled.
    while(size(connections) != 0) {
      printf("Waiting on pending connections.\n");
      sleep(1);
    }

    // Close and clean. 
    destroyQueue(connections);
    destroy_cs(connections_cs);
    close(sock);
    stop_logging();
    destroy_thread_pool();
    destroy_semaphore();

    // If windows stop WSA and close socket with win api.
#ifdef _WIN32
    WSACleanup();
#endif

    printf("Stopped. S\n\n");
  }

  return 0;
}

