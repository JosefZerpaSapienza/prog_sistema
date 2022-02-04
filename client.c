#include <stdio.h>
#include <stdlib.h>

#define USAGE "Usage: a.out -h <ip_addr> -p <port> <cmd> \n\n"

int main(int argc, char **argv) 
{
  char *ip;
  int port;
  char *cmd;

  if(argc < 6) 
  {
    printf(USAGE);
    return -1;
  }
  get_parameters(argc, argv, &ip, &port, &cmd);

}

// Return 0 on success, -1 otherwise.
int get_parameters(
        int argc, char **argv, char **ip, int *port, char **cmd)
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
        case 'h':
		*ip = option[i + 1];
                i++;
                break;
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
        case 'l':
                *conf_file = argv[i + 1];
                i++;
                break;

