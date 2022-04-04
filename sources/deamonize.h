// Deamonize a process
//

// Deamonize a process.
int deamonize() {
  pid_t pid = fork();
  // Check failure
  if (pid < 0) {
    return -1;
  }
  // Kill parent process
  if (pid > 0) {
    printf("process_id: %d \n", pid);
    exit(0);
  }

  pid_t sid = 0;
  // Check failure
  if (sid < 0) {
    return -1;
  }

  // TODO: Continue
  // Set / Close stdin stdout.
  // Check working dir and umask.
}
