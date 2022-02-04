/* Chapter 1. Basic cp file copy program. Win32 Implementation.
	FAST VERSION USING
		1.  LARGE BUFFER
		2.  PRE-SET OUTPUT FILE SIZE
		3.  SEQUENTIAL SCAN */
/* cp file1 file2: Copy file1 to file2. */

#include <windows.h>
#include <stdio.h>
#define BUF_SIZE 8192

int main (int argc, LPTSTR argv [])
{
	HANDLE hIn, hOut;
	DWORD nIn, nOut, FsLow;
	CHAR Buffer [BUF_SIZE];
	if (argc != 3) {
		printf ("Usage: cp file1 file2\n");
		return 1;
	}
	hIn = CreateFile (argv [1], GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		printf ("Cannot open input file. Error: %x\n", GetLastError ());
		return 2;
	}
	hOut = CreateFile (argv [2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf ("Cannot open output file. Error: %x\n", GetLastError ());
		return 3;
	}

	//FsLow = GetFileSize (hIn, NULL);
	//SetFilePointer (hOut, FsLow, NULL, FILE_BEGIN);
	//SetEndOfFile (hOut);

	/*  Set the output file size. */

	while (ReadFile (hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
		WriteFile (hOut, Buffer, nIn, &nOut, NULL);
		if (nIn != nOut) {
			printf ("Fatal write error: %x\n", GetLastError ());
			return 4;
		}
	}
	CloseHandle (hIn);
	CloseHandle (hOut);
	return 0;
}
