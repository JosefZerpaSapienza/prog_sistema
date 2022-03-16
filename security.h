// Security utility functions. 
//
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Hash function.
// Taken from: http://www.cse.yorku.ca/~oz/hash.html
uint64_t hash(unsigned char *str)
{
  // using int because long seems to be platform dependent 
  // TODO: Check use of long.
  uint64_t hash = 5381;
  int c;

  while (c = *str++) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

// Returns a unique token
uint64_t generateToken(char *passphrase) {
  return  hash(passphrase);
}

// Set rand() seed.
void set_rand_seed() {
  // Only run once.
  static int run = 1;
  if (run) {
    srand(time(0));
    run = 0;
  }
}

// Generate a pseudo-random 64 bits number.
uint64_t long_rand() {
  uint64_t random = 0;
  uint64_t shift = 8;
  uint64_t mask = 255;
  uint64_t temp;
  int size = sizeof(uint64_t) * 8;

  // Set seed. Randomize between executions.
#ifdef __linux__
  set_rand_seed();
#elif defined _WIN32
  srand(time(0));
#endif
  // Build up a 64 bits number with subsequent calls to rand().
  for(int i = 0; i < size; i += shift) {
    temp = (uint64_t) rand();
    temp = temp & mask;
    random = random << shift;
    random = random | temp;
  }

  return random;
}

