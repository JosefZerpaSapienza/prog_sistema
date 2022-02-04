/* "Fast" implementation of asc2un.c with all combinations of a Large Buffer (LB),
 *	Sequential Scan Flags (SS), and setting the output file size (FS). 
 *	NB - no buffering
 *	FL - Flush buffers at file at end. */
/* This allows experimentation with a wide variety of techniques to improve
 * Sequential file processing performance. */

#define LB
//#undef LB
#define SS
//#undef SS
//#define FS
//#define NB

#include "EvryThng.h"

/* BigBuffer Defined? */

#ifdef LB
#define BUF_SIZE 8192
#endif
#ifndef LB
#define BUF_SIZE 256
#endif

/* Set sequential scan flags? */

#ifdef SS
#define SSF FILE_FLAG_SEQUENTIAL_SCAN
#endif
#ifndef SS
#define SSF 0
#endif

/* Buffer I/O */
#ifdef NB
#define NBF FILE_FLAG_NO_BUFFERING
#endif
#ifndef NB
#define NBF 0
#endif

/* Flush buffers at end */
#define FL

/* A refinement that attempts to speed up the Win32 file processing by:
	Using a larger buffer
	Presizing the output file
	Using the sequential scan flag on the two files. */

BOOL Asc2Un (LPCTSTR fIn, LPCTSTR fOut,  BOOL bFailIfExists)

/* ASCII to Unicode file copy function
		fIn:		Source file path name
		fOut:		Destination file path name
		bFailIfExists:	Do not copy if the destination file already exists
		Behavior is modeled after CopyFile. */
{
	HANDLE hIn, hOut;
	DWORD fdwOut, nIn, nOut, iCopy, FsLow;
	CHAR aBuffer [BUF_SIZE];
	WCHAR uBuffer [BUF_SIZE];
	BOOL WriteOK = TRUE;

	hIn = CreateFile (fIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL | SSF | NBF, NULL);
	if (hIn == INVALID_HANDLE_VALUE) return FALSE;
	FsLow = GetFileSize (hIn, NULL);

	fdwOut = bFailIfExists ? CREATE_NEW : CREATE_ALWAYS;
	hOut = CreateFile (fOut, GENERIC_WRITE, 0, NULL, fdwOut,
			FILE_ATTRIBUTE_NORMAL | SSF | NBF, NULL);
	if (hOut == INVALID_HANDLE_VALUE) return FALSE;

#ifdef FS
	SetFilePointer (hOut, 2 * FsLow, NULL, FILE_BEGIN);
	SetEndOfFile (hOut);
	SetFilePointer (hOut, 0, NULL, FILE_BEGIN);
#endif

	while (ReadFile (hIn, aBuffer, BUF_SIZE, &nIn, NULL) && nIn > 0 && WriteOK) {
		for (iCopy = 0; iCopy < nIn; iCopy++)
			uBuffer [iCopy] = (WCHAR) aBuffer [iCopy];
		WriteOK = WriteFile (hOut, uBuffer, 2 * nIn, &nOut, NULL);
	}

#ifdef FL
	FlushFileBuffers (hOut);
#endif
	CloseHandle (hIn);
	CloseHandle (hOut);

	return WriteOK;
}
