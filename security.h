// Authentication functions.
//

// Hash function.
// Taken from: http://www.cse.yorku.ca/~oz/hash.html
unsigned long int hash(unsigned char *str)
{
  // using int because long seems to be platform dependent
  unsigned int hash = 5381;
  int c;

  while (c = *str++)
  {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return (unsigned long) hash;
}

// Returns a unique token
unsigned int generateToken(char *passphrase)
{
  return  hash(passphrase);
}
