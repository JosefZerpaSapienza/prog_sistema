#ifdef __linux__
  #include <pthread.h>
  #include <semaphore.h>
#elif defined _WIN32
  #include <Windows.h>
  #define SEM_MAX 100
#endif

#ifdef __linux__
  static sem_t semaphore; // Check variable scope
#elif defined _WIN32
  static HANDLE semaphore;
#endif

// Return -1 on failure.
int  create_semaphore() {
#ifdef __linux__
  return sem_init(&semaphore, 0, 0);
#elif defined _WIN32
  semaphore = CreateSemaphore(NULL, 0, SEM_MAX, NULL);
  if (semaphore == NULL) { return -1; }
  return 0;
#endif
}

// Return -1 on failure.
int increment_semaphore() {
#ifdef __linux__
  return sem_post(&semaphore);
#elif defined _WIN32
  if(ReleaseSemaphore(semaphore, 1, NULL) != 0) { return 0; }
  return -1;
#endif
}

// Return -1 on failure.
int wait_semaphore() {
#ifdef __linux__
  return sem_wait(&semaphore);
#elif defined _WIN32
  if(WaitForSingleObject(semaphore, INFINITE) == WAIT_FAILED) { 
    printf("%d\n", GetLastError()); 
    return -1; 
  }
  return 0;
#endif
}

// Return -1 on failure.
int destroy_semaphore() {
#ifdef __linux__
  return sem_destroy(&semaphore);
#elif defined _WIN32
  if(CloseHandle(semaphore) != 0) { return 0; }
  return -1;
#endif
}

// Dynamically allocate and return a critical section variable.
// Return NULL on failure.
void *create_cs() {
#ifdef __linux__
  pthread_mutex_t *cs = malloc(sizeof(pthread_mutex_t));
  if (cs == NULL) { return NULL; }
  if (pthread_mutex_init(cs, NULL) != 0) { return NULL; }
  return (void *) cs;
#elif defined _WIN32
  CRITICAL_SECTION *cs = malloc(sizeof(CRITICAL_SECTION));
  if (cs == NULL) { return NULL; }
  InitializeCriticalSection(cs);
  return (void *) cs;
#endif
}

// Enter critical section.
// Return OK on success,
// INT_ERR on failure.
int enter_cs(void *cs) {
#ifdef __linux__
  if (pthread_mutex_lock((pthread_mutex_t *) cs) != 0) {
	  return INT_ERR;
  }
#elif defined _WIN32
  EnterCriticalSection((CRITICAL_SECTION *) cs);
#endif
  return OK;
}

// Leave critical section.
// Return OK on success,
// INT_ERR on failure.
int leave_cs(void *cs) {
#ifdef __linux__
  if (pthread_mutex_unlock((pthread_mutex_t *) cs) != 0) {
	  return INT_ERR;
  }
#elif _WIN32
  LeaveCriticalSection((CRITICAL_SECTION *) cs);
#endif
  return OK;
}

// Destroy critical section and free memory.
void destroy_cs(void *cs) {
#ifdef __linux__
  pthread_mutex_destroy((pthread_mutex_t *) cs);
#elif _WIN32
  DeleteCriticalSection((CRITICAL_SECTION *) cs);
#endif
  free(cs);
}
