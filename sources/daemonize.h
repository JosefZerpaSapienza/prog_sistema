// Daemonize a process
//
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "constants.h"

// Daemonize a process and redirect its stdout and stderr.
int daemonize() {
  // Fork.
  pid_t pid = fork();
  // Check failure
  if (pid < 0) {
    return INT_ERR;
  }

  // Kill parent process
  if (pid > 0) {
    exit(0);
  }

  // Set new session.
  pid_t sid = setsid();
  // Check failure
  if (sid < 0) {
    return INT_ERR;
  }

  // Fork again: make it not session leader.
  struct sigaction action;
  action.sa_handler = SIG_IGN;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGHUP, NULL, &action);
  pid = fork();
  // Check failure
  if (pid < 0) {
    return INT_ERR;
  }

  // Kill parent process again.
  if (pid > 0) {
    printf("process_id: %d \n", pid);
    exit(0);
  }

  // Redirect stdout and stderr to REDIRECTION_FILE_PATH.
  int fd = open(REDIRECTION_FILE_PATH, O_RDWR|O_CREAT|O_APPEND, 0644);
  if (fd == -1) {
	perror("Could not open redirection file.\n");
	return INT_ERR;
  }
  dup2(fd, STDOUT_FILENO);
  dup2(fd, STDERR_FILENO);
  close(fd);
  close(STDIN_FILENO);

  // Check working dir and umask.
  return OK;
}

