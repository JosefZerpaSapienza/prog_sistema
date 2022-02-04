/* Chapter 14. atouMTCP
	Multithreaded ASCII to Unicode file conversion.
	Use threads and I/O completion ports with overlapped I/O. */
/* atouMTCP nOvr file1 file2 */
/* nOvr is the number of overlapped operations, < MAX_OVRLP */

#include "EvryThng.h"

#define MAX_OVRLP 8
#define REC_SIZE 0x2000
#define UREC_SIZE 2 * REC_SIZE

static DWORD WINAPI ReadWrite (LPVOID), nOvr;
static HANDLE hIn [MAX_OVRLP], hOut [MAX_OVRLP];
static LARGE_INTEGER FileSize;

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInFile, hOutFile, hThr [MAX_OVRLP];
	HANDLE hCompPort; /* I/O completion port handle. */
	HANDLE hProc = GetCurrentProcess ();
	DWORD i, ThId;

	if (argc != 4)
		ReportError (_T ("Usage: atouMTCP nOvr file1 file2"), 1, FALSE);

	nOvr = _ttoi (argv [1]);
	if (nOvr > MAX_OVRLP)
		ReportError (_T ("nOvr is too large"), 2, FALSE);

	/* Input and output files are both overlapped. */

	hInFile  = CreateFile (argv [2], GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
 	if (hInFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening input file."), 3, TRUE);

	/* Get the file size. */

	FileSize.LowPart = GetFileSize (hInFile, &FileSize.HighPart);

	hOutFile = CreateFile (argv [3], GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (hOutFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening output file."), 4, TRUE);

	/* Create the new I/O completion port to be associated with
		the duplicated file handles. */

	hCompPort = CreateIoCompletionPort (
		INVALID_HANDLE_VALUE, NULL, 2 * nOvr, nOvr);
	if (hCompPort == NULL)
		ReportError (_T ("Failed to create input completion port."), 5, TRUE);

	/* Duplicate input and output handles for each thread
		and create the thread. The only thread argument is
		its number (0, 1, 2, ..., MAX_OVRLP-1). The thread
		gets everything else it needs from global storage. 
		Add the duplicate handles to the I/O completion port. */

	for (i = 0; i < nOvr; i++) {
		hIn [i] = CreateFile (argv [2], GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
//		Duplicated handles, created as follows, DOES NOT WORK.
//		if (!DuplicateHandle (hProc, hInFile, hProc, &hIn [i],
//			0, FALSE, DUPLICATE_SAME_ACCESS))
//			ReportError (_T ("Error duplicating input handle."), 6, TRUE);
		if (hIn[i] == INVALID_HANDLE_VALUE)
			ReportError (_T ("Error opening duplicate input handle."), 7, TRUE);

 		if (CreateIoCompletionPort (hIn [i], hCompPort, i, nOvr) == NULL)
			ReportError (_T ("Failure adding dup input handle to comp port."),
				8, TRUE);

		hOut [i] = CreateFile (argv [3], GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		if (hOut[i] == INVALID_HANDLE_VALUE)
			ReportError (_T ("Error opening duplicate output handle."), 9, TRUE);
//		Duplicated handles, created as follows, DOES NOT WORK.
//		if (!DuplicateHandle (hProc, hOutFile, hProc, &hOut [i],
//			0, FALSE, DUPLICATE_SAME_ACCESS))
//			ReportError (_T ("Error duplicating output handle."), 10, TRUE);

		if (CreateIoCompletionPort
				(hOut [i], hCompPort, i + nOvr, nOvr) == NULL)
			ReportError (_T ("Failure adding dup output handle to comp port."),
					11, TRUE);

		/* Create the threads to do the I/O. */

		hThr [i] = (HANDLE) _beginthreadex (NULL, 0, ReadWrite, (LPVOID) i, 0, &ThId);
		if (hThr [i] == NULL)
			ReportError (_T ("Error creating thread."), 12, TRUE);
	}

	/*  Wait for all the threads to terminate. */

	WaitForMultipleObjects (nOvr, hThr, TRUE, INFINITE);
	for (i = 0; i < nOvr; i++)
		CloseHandle (hThr [i]);
	CloseHandle (hCompPort);

	_tprintf (_T ("ASCII to Unicode conversion completed.\n"));
	return 0;
}

static DWORD WINAPI ReadWrite (LPVOID ipTh)
{
	CHAR AsRec [REC_SIZE];
	WCHAR UnRec [REC_SIZE];
	BOOL Exit = FALSE;
	LARGE_INTEGER CurPosIn, CurPosOut;
	DWORD i, nRead = 1, nWrite, iTh = (DWORD)ipTh;
	OVERLAPPED ov = {0, 0, 0, 0, NULL};
	HANDLE hEv;

	/* Create an event for the overlapped I/O structure. */

	hEv = ov.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
	if (hEv == NULL)
		ReportError (_T ("Error Creating event."), 13, TRUE);

	/*  Set the event's low-order bit to allow I/O completion
		synchronization on it rather than getting queued status. */
	ov.hEvent = (HANDLE) ((DWORD)hEv | 0x1);
	CurPosIn.QuadPart  = (LONGLONG) REC_SIZE * iTh;
	CurPosOut.QuadPart = (LONGLONG) REC_SIZE * iTh * 2;
	while (nRead > 0 && CurPosIn.QuadPart < FileSize.QuadPart) {

		/* Set input file position in the overlapped structure. */

		ov.Offset     = CurPosIn.LowPart;
		ov.OffsetHigh =	CurPosIn.HighPart;
		ReadFile (hIn [iTh], AsRec, REC_SIZE, &nRead, &ov);

		/* Wait for the I/O operation to complete. */

		WaitForSingleObject (ov.hEvent, INFINITE);
		if (!GetOverlappedResult (hIn [iTh], &ov, &nRead, FALSE))
			ReportError (_T ("Error getting ov result."), 14, TRUE);

		/* Convert the record to ASCII. */

		for (i = 0; i < nRead; i++)
			UnRec [i] = AsRec [i];

		/* Set output file position. */

		ov.Offset     = CurPosOut.LowPart;
		ov.OffsetHigh =	CurPosOut.HighPart;
		WriteFile (hOut [iTh], UnRec, nRead * 2, &nWrite, &ov);
		WaitForSingleObject (ov.hEvent, INFINITE);

		/* Update the file positions. */

		CurPosIn.QuadPart  += (LONGLONG) REC_SIZE * nOvr;
		CurPosOut.QuadPart += (LONGLONG) REC_SIZE * 2 * nOvr;
	}

	CloseHandle (hIn [iTh]);
	CloseHandle (hOut [iTh]);
	CloseHandle (hEv);
	_endthreadex (0);
	return 0;
}