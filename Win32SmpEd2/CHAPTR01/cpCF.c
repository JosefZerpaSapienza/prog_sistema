/* Chapter 1. Basic cp file copy program.
	Win32 Implementation using CopyFile for convenience and possible performance. */
/* cp file1 file2: Copy file1 to file2. */

#include <windows.h>
#include <stdio.h>
#define BUF_SIZE 256

int main (int argc, LPTSTR argv [])
{
	if (argc != 3) {
		printf ("Usage: cp file1 file2\n");
		return 1;
	}
	if (!CopyFile (argv [1], argv [2], FALSE)) {
		printf ("CopyFile Error: %x\n", GetLastError ());
		return 2;
	}
	return 0;
}
