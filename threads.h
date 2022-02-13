#ifdef __linux__
  #include <pthread.h>
#elif defined _WIN32
  #include <Windows.h>
#endif

// Launches a pool of n threads, executing the exec function.
void create_thread_pool(int n, void *exec) {
  for(int i = 0; i < n; i++) {
#ifdef __linux__
    pthread_t foo;
    pthread_create(&foo, NULL,  exec, NULL);
#elif defined _WIN32
    CreateThread(NULL, 0, exec, NULL, 0, NULL);
#endif
  }
}
