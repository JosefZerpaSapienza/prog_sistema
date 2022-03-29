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
void create_thread_pool(int n, void *exec) {
  // Destroy any previously created pool..
  if (pool != NULL) {
    for (void *thread = dequeue(pool); thread != NULL; thread = dequeue(pool)) {
      // Kill thread.
      // TODO: implement a nicer way to kill threads.
#ifdef __linux__
      pthread_kill(*((pthread_t *)thread), 9);
#elif defined _WIN32
      TerminateThread(thread, 0); 
#endif
    }
    destroyQueue(pool);
  }

  // Create new pool.
  pool = createQueue(n);

  // Add threads.
  for(int i = 0; i < n; i++) {
#ifdef __linux__
    //pthread_create((pthread_t *) thread, NULL,  exec, NULL);
    pthread_t thread;
    pthread_create(&thread, NULL,  exec, NULL);
#elif defined _WIN32
    HANDLE thread;
    thread = CreateThread(NULL, 0, exec, NULL, 0, NULL);
#endif
    enqueue(pool, &thread);
  }
}

int get_thread_id() {
#ifdef __linux__
  return syscall(__NR_gettid); 
#elif defined _WIN32
  return GetCurrentThreadId();
#endif
}
