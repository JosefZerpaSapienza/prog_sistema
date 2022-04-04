#include "queue.h"
#ifdef __linux__
  #include <signal.h>
  #include <sys/types.h>
  #include <sys/syscall.h>
  #include <pthread.h>
  #include <unistd.h>
#elif defined _WIN32
  #include <Windows.h>
#endif

static struct Queue *pool = NULL;

// Creates a pool of n threads, executing the exec function.
int create_thread_pool(int n, void *exec) {
  // Destroy any previously created pool..
  if (pool != NULL) {
    for (void *thread = dequeue(pool); thread != NULL; thread = dequeue(pool)) {
      // Kill thread.
      // TODO: implement a nicer way to terminate threads.
#ifdef __linux__
      pthread_kill(*((pthread_t *)thread), 9);
#elif defined _WIN32
      TerminateThread(thread, 0); 
#endif
      free(thread);
    }
    destroyQueue(pool);
  }

  // Create new pool.
  pool = createQueue(n);
  if (pool == NULL) {
	  printf("Could not create queue. \n");
	  return INT_ERR;
  }

  // Add threads.
  for(int i = 0; i < n; i++) {
#ifdef __linux__
	// Allocate thread.
    pthread_t *thread = malloc(sizeof(pthread_t));
    if (thread == NULL) {
      printf("Could not malloc. \n");
      return INT_ERR;
    }
    // Initialize thread.
    if (pthread_create(thread, NULL,  exec, NULL) != 0 {
      perror("Could not create thread.");
      return INT_ERR;
    }
#elif defined _WIN32
	// Allocate thread.
    HANDLE *thread = malloc(sizeof(HANDLE));
    if (thread == NULL) {
      printf("Could not malloc. \n");
      return INT_ERR;
    }
    // Initialize thread.
    *thread = CreateThread(NULL, 0, exec, NULL, 0, NULL);
    if (*thread == NULL) {
      perror("Could not create thread.");
      return INT_ERR;
    }
#endif
    // Enqueue thread pointer.
    if (enqueue(pool, thread) == -1) {
      printf("Could not enqueue new thread. \n");
      return INT_ERR;
    }
  }

  return OK;
}

int get_thread_id() {
#ifdef __linux__
  return syscall(__NR_gettid); 
#elif defined _WIN32
  return GetCurrentThreadId();
#endif
}
