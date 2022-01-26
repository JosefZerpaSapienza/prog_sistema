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
#define CONF_ARRAY_SIZE 10

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

  // Read parameters
  if(conf_file) { update_parameters(conf_file, &port, &n_threads); }

  // Get passphrase
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

