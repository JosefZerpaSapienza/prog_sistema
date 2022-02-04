/* Chapter 3. getn command. */
/* get file      
	Get a specified fixed size records from the file.
	The user is prompted for record numbers. Each
	requested record is retrieved and displayed
	until a negative record number is requested.
	Fixed size, text records are assumed. */
/*	There is a maximum line size (MAX_LINE_SIZE). */

/* This program illustrates:
	1. Setting the file pointer.
	2. LARGE_INTEGER arithmetic and using the 64-bit file positions. */

#include "EvryThng.h"

	/* One more than number of lines in the tail. */

#define MAX_LINE_SIZE 256

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInFile;
	LARGE_INTEGER CurPtr;
	DWORD nRead, FPos, RecSize;
	TCHAR Buffer [MAX_LINE_SIZE + 1];
	BOOL Exit = FALSE;
	LPTSTR p;
	int RecNo;

 	if (argc != 2)
		ReportError (_T ("Usage: getn file"), 1, FALSE);
	hInFile = CreateFile (argv [1], GENERIC_READ,
			0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hInFile == INVALID_HANDLE_VALUE)
		ReportError (_T ("getn error: Cannot open file."), 2, TRUE);
	
	RecSize = 0;
		/* Get the record size from the first record.
			Read the max line size and look for a CR/LF. */

	if (!ReadFile (hInFile, Buffer, sizeof (Buffer), &nRead, NULL) || nRead == 0)
		ReportError (_T ("Nothing to read."), 1, FALSE);
	if ((p = _tstrstr (Buffer, _T ("\r\n"))) == NULL)
		ReportError (_T ("No end of line found."), 2, FALSE);
	RecSize = p - Buffer + 2;	/* 2 for the CRLF. */

	_tprintf (_T ("Record size is: %d\n"), RecSize);
	
	while (TRUE) {
		_tprintf (_T ("Enter record number. Negative to exit: "));
		_tscanf (_T ("%d"), &RecNo);
		if (RecNo < 0) break;

		CurPtr.QuadPart = (LONGLONG) RecNo * RecSize;
		FPos = SetFilePointer (hInFile, CurPtr.LowPart, &CurPtr.HighPart, FILE_BEGIN);
		/* Alternative: Use an overlapped structure */
		if (FPos == 0xFFFFFFFF && GetLastError () != NO_ERROR)
			ReportError (_T ("getn Error: Set Pointer."), 3, TRUE);
		if (!ReadFile (hInFile, Buffer, RecSize,  &nRead, NULL) || (nRead != RecSize))
			ReportError (_T ("Error reading n-th record."), 0, TRUE);
		Buffer [RecSize] = '\0';
		_tprintf (_T ("%s\n"), Buffer);
	}
	CloseHandle (hInFile);
	return 0;
}
