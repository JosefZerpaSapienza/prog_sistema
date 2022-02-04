/* Asc2UnDll.c  */
/* Asc2Un.c, Chapter 2, modified to build as a DLL */
#include "EvryThng.h"

#define BUF_SIZE 256

/* The following line [_declspec (dllexport)] is the only change from Asc2Un.c */
/* In general, you should have a single function and use a preprocessor */
/* variable to determine if the function is exported or not */

_declspec (dllexport)
BOOL Asc2Un (LPCTSTR fIn, LPCTSTR fOut, BOOL bFailIfExists)

/* ASCII to Unicode file copy function  - Simple implementation
 *		fIn:		Source file pathname
 *		fOut:		Destination file pathname
 *		bFailIfExists:	Do not copy if the destination file already exists
 *	Behavior is modeled after CopyFile */
{
	HANDLE hIn, hOut;
	DWORD fdwOut, nIn, nOut, iCopy;
	CHAR aBuffer [BUF_SIZE];
	WCHAR uBuffer [BUF_SIZE];
	BOOL WriteOK = TRUE;

	hIn = CreateFile (fIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hIn == INVALID_HANDLE_VALUE) return FALSE;

	fdwOut = bFailIfExists ? CREATE_NEW : CREATE_ALWAYS;

	hOut = CreateFile (fOut, GENERIC_WRITE, 0, NULL, fdwOut, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hOut == INVALID_HANDLE_VALUE) return FALSE;

	while (ReadFile (hIn, aBuffer, BUF_SIZE, &nIn, NULL) && nIn > 0 && WriteOK) {
		for (iCopy = 0; iCopy < nIn; iCopy++)
			uBuffer [iCopy] = (WCHAR) aBuffer [iCopy];
		WriteOK = WriteFile (hOut, uBuffer, 2 * nIn, &nOut, NULL);
	}

	CloseHandle (hIn);
	CloseHandle (hOut);

	return WriteOK;
}
