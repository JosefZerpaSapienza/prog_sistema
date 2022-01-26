#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "security.h"

#define DEFAULT_PORT 8888
#define DEFAULT_N_THREADS 10
#ifdef __linux__
  #define DEFAULT_LOG_FILE "/tmp/server.log"
#elif defined _WIN32
  #define DEFAULT_LOG_FILE "C:\\\\Windows\\Temp\\server.log"
#endif
#define PASSPHRASE_BUFFER_SIZE 256

// Get command line parameters.
int get_parameters(
	int argc, char **argv, int *port, int *n_threads, 
	char **conf_file, int *s, char **log_file);
//
void log_to_file(char *message, char *filename);

int main (int argc, char **argv) 
{
  int port = DEFAULT_PORT;
  int n_threads = DEFAULT_N_THREADS;
  char *conf_file = NULL;
  int s = 0;
  char *log_file = DEFAULT_LOG_FILE;
  char passphrase[PASSPHRASE_BUFFER_SIZE];

  // Get parameters and check errors.
  if(get_parameters(argc, argv, &port, &n_threads, &conf_file, &s, &log_file))
  {
    return -1;
  }

  printf("Type passphrase: ");
  printf("\n");//scanf("%s", passphrase);
  unsigned long token = generateToken(passphrase);
  memset(passphrase, 0, PASSPHRASE_BUFFER_SIZE);

  if(s)
  {
    printf("token: %lu \n", token);
  }


  return 0;
}

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

  for(int i = 1; i < argc; i++)
  {
    option = argv[i];
    if(option[0] == '-')
    {
      switch(option[1]) 
      {
        case 'p':
		*port = atoi(argv[i + 1]);
		if (*port == 0) 
		{ 
		  perror("Error: port not acceptable.\n"); 
		  return -1; 
		}
		i++;
		break;
	case 'n':
		*n_threads = atoi(argv[i + 1]);
		if (*n_threads == 0) 
		{ 
		  perror("Error: number of threads not acceptable.\n"); 
		  return -1; 
		}
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
		perror("Parameter not supported.");
		return -1;
      }
    }
  }

  return 0;
}
