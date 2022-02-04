/* Asc2UnMMDll.c. Chapter 6  */
/* Asc2UnMM.c, Chapter 6, modified to build as a DLL */
/* Chapter 6.
	asc2unMM.c function: Memory Mapped implementation of the
	ASCII to UNICODE file copy function. */

#include "EvryThng.h"
/* The following line [_declspec (dllexport)] is the only change from Asc2UnMM.c */
/* In general, you should have a single function and use a preprocessor */
/* variable to determine if the function is exported or not */

_declspec (dllexport)
BOOL Asc2Un (LPCTSTR fIn, LPCTSTR fOut, BOOL bFailIfExists)

/* ASCII to Unicode file copy function. 
 *	fIn:		Source file pathname.
 *	fOut:		Destination file pathname.
 *	bFailIfExists:	Do not copy if the destination file already exists.
 *		Behavior is modeled on CopyFile. */
{
BOOL Complete = FALSE;
_try {
	HANDLE hIn, hOut;
	HANDLE hInMap, hOutMap;
	LPSTR pIn, pInFile;
	LPWSTR pOut, pOutFile;
	DWORD FsLow, fdwOut;

	/* Open the input file. */
	hIn = CreateFile (fIn, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) 
		ReportException (_T ("Failure opening input file."), 1);

	/* Create a file mapping object on the input file. Use the file size. */
	hInMap = CreateFileMapping (hIn, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hInMap == INVALID_HANDLE_VALUE)
		ReportException (_T ("Failure Creating input map."), 2);

	/* Map the input file */
	pInFile = MapViewOfFile (hInMap, FILE_MAP_READ, 0, 0, 0);
	if (pInFile == NULL)
		ReportException (_T ("Failure Mapping input file."), 3);

	/* Get the input file size. As the mapping succeeded, the file size is < 2 GB. */
	FsLow = GetFileSize (hIn, NULL);
	if (FsLow == 0xFFFFFFFF)
		ReportException (_T ("Failure getting file size."), 4);

	/*  Create/Open the output file. */
	fdwOut = bFailIfExists ? CREATE_NEW : CREATE_ALWAYS;

	/* The output file MUST have Read/Write access for the mapping to succeed. */
	hOut = CreateFile (fOut, GENERIC_READ | GENERIC_WRITE,
			0, NULL, fdwOut, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		Complete = TRUE; /* Do not delete an existing file. */
		ReportException (_T ("Failure Opening output file."), 5);
	}

	/* Set the output file's size to twice the input size. */
	/* This commented code is included for illustration and as an alterantive */
	/* to setting the file size in the CreateFileMapping call */
/*	if ( SetFilePointer (hOut, 2 * FsLow, NULL, FILE_BEGIN) == 0xFFFFFFFF
			|| !SetEndOfFile (hOut))
		ReportException (_T ("Failure setting file size."), 6);
*/
	/* Map the output file. Using the file length in the next two calls
		is optional but must be done if you do not set the file size above.
		In fact, using the file size on CreateFileMapping will expand
		the file if it is smaller than the mapping. */

	hOutMap = CreateFileMapping (hOut, NULL, PAGE_READWRITE, 0, 2 * FsLow, NULL);
	if (hOutMap == INVALID_HANDLE_VALUE)
		ReportException (_T ("Failure creating output map."), 7);
	pOutFile = MapViewOfFile (hOutMap, FILE_MAP_WRITE, 0, 0, (SIZE_T)(2 * FsLow));
	if (pOutFile == NULL)
		ReportException (_T ("Failure mapping output file."), 8);

	/* Now move the input file to the output file, doing all the work in memory. */

	pIn = pInFile;
	pOut = pOutFile;
	while (pIn < pInFile + FsLow) {
		*pOut = (WCHAR) *pIn;
		pIn++;
		pOut++;
	}

	/* Close all views and handles. */

	UnmapViewOfFile (pOutFile); UnmapViewOfFile (pInFile);
	CloseHandle (hOutMap); CloseHandle (hInMap);
	CloseHandle (hIn); CloseHandle (hOut);
	Complete = TRUE;
	return TRUE;
}

_except (EXCEPTION_EXECUTE_HANDLER) {
		/* Delete the output file if the operation did not complete successfully. */
	if (!Complete)
		DeleteFile (fOut);
	return FALSE;
}
}




