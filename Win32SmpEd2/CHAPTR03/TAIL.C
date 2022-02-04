/* Chapter 3. tail command. */
/* tail file - Print the last 10 lines of the named file.
	All options are ignored. The file must be specified. */

/* This program illustrates:
	1. Getting the file size.
	2. Setting the file pointer.
	3. LARGE_INTEGER arithmetic and using the 64-bit file positions.
	4. Getting the current file position by moving
		0 bytes from the current position. */

#include "EvryThng.h"

#define NUM_LINES 11
	/* One more than number of lines in the tail. */
#define MAX_LINE_SIZE 256
#define MAX_CHAR NUM_LINES*MAX_LINE_SIZE

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInFile;
	HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);
	LARGE_INTEGER FileSize, CurPtr;
	LARGE_INTEGER LinePos [NUM_LINES];
	DWORD LastLine, FirstLine, LineCount, nRead, FPos;
	TCHAR Buffer [MAX_CHAR + 1], c;

	if (argc != 2)
		ReportError (_T ("Usage: tail file"), 1, FALSE);
	hInFile = CreateFile (argv [1], GENERIC_READ,
			0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hInFile == INVALID_HANDLE_VALUE)
		ReportError (_T ("tail error: Cannot openfile."), 2, TRUE);

	/* Get the current file size. */

	FileSize.LowPart = GetFileSize (hInFile, &FileSize.HighPart);
	if (FileSize.LowPart == 0xFFFFFFFF && GetLastError () != NO_ERROR)
		ReportError (_T ("tail error: file size"), 3, TRUE);

	/* Set the file pointer on the assumption that 256
		is the maximum size and look for the line beginnings.
		If 10 lines are not located, then just put out the
		ones that are found. A more general solution would look
		farther back in the file if necessary. Alternatively,
		start the scan at the beginning of the file, but that
		would be time consuming for a large file. */

	CurPtr.QuadPart = (LONGLONG)FileSize.QuadPart
			- NUM_LINES * MAX_LINE_SIZE * sizeof (TCHAR);
	if (CurPtr.QuadPart < 0) CurPtr.QuadPart = 0;
	FPos = SetFilePointer (hInFile, CurPtr.LowPart, &CurPtr.HighPart, FILE_BEGIN);
	if (FPos == 0xFFFFFFFF && GetLastError () != NO_ERROR)
		ReportError (_T ("tail Error: Set Pointer."), 4, TRUE);

	/*  Scan the file for the start of new lines and retain their
		position. Assume that a line starts at the current position. */

	LinePos [0].QuadPart = CurPtr.QuadPart;
	LineCount = 1;
	LastLine = 1;
	while (TRUE) {
		while (ReadFile (hInFile, &c, sizeof (TCHAR), &nRead, NULL)
				&& nRead > 0 && c != CR) ; /* Empty loop body */
		if (nRead < sizeof (TCHAR)) break;
					/* Found a CR. Is LF next? */
		ReadFile (hInFile, &c, sizeof (TCHAR), &nRead, NULL);
		if (nRead < sizeof (TCHAR)) break;
		if (c != LF) continue;
		CurPtr.QuadPart = 0;
				/* Get the current file position. */
		CurPtr.LowPart = SetFilePointer (
				hInFile, 0, &CurPtr.HighPart, FILE_CURRENT);
				/* Retain the start-of-line position */
		LinePos [LastLine].QuadPart = CurPtr.QuadPart;
		LineCount++;
		LastLine = LineCount % NUM_LINES;
	}
	
	FirstLine = LastLine % NUM_LINES;
	if (LineCount < NUM_LINES) FirstLine = 0;
	CurPtr.QuadPart = LinePos [FirstLine].QuadPart;

	/* The start of each line is now stored in LinePos []
		with the last line having index LastLine.
		Display the last strings. */

	SetFilePointer (hInFile, CurPtr.LowPart, &CurPtr.HighPart, FILE_BEGIN);
	ReadFile (hInFile, Buffer, sizeof (Buffer), &nRead, NULL);
	Buffer [nRead] = '\0';
	PrintMsg (hStdOut, Buffer);
	CloseHandle (hInFile);
	return 0;
}
