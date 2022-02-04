/* Chapter 14. atouEX.   EXTENDED I/O ASCII to Unicode file conversion. */
/*	atouEX file1 file2  */
/* This program illustrates Extended (a.k.a.
	alterable or completion routine asynch) I/O.
	It was developed by restructuring atouEX. */

#include "EvryThng.h"

#define MAX_OVRLP 4
#define RECORD_SIZE 0x2000 /* Block size is not as important for performance as with atouOV */
#define URECORD_SIZE 2 * RECORD_SIZE

static VOID WINAPI ReadDone (DWORD, DWORD, LPOVERLAPPED);
static VOID WINAPI WriteDone (DWORD, DWORD, LPOVERLAPPED);

/* The first overlapped structure is for reading,
	and the second is for writing. Structures and buffers are
	allocated for each oustanding operation */

OVERLAPPED OverLapIn [MAX_OVRLP], OverLapOut [MAX_OVRLP];
CHAR AsRec [MAX_OVRLP] [RECORD_SIZE];
WCHAR UnRec [MAX_OVRLP] [RECORD_SIZE];
HANDLE hInputFile, hOutputFile;
LONGLONG nRecord, nDone;
LARGE_INTEGER FileSize;


int _tmain (int argc, LPTSTR argv [])
{
	DWORD ic;
	LARGE_INTEGER CurPosIn;

	if (argc != 3)
		ReportError (_T ("Usage: atouEX file1 file2"), 1, FALSE);

	hInputFile = CreateFile (argv [1], GENERIC_READ,
			0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hInputFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening input file."), 2, TRUE);
	
	hOutputFile = CreateFile (argv [2], GENERIC_WRITE,
			0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hOutputFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening output file."), 3, TRUE);

	/* Compute the total number of records to be processed
		from the input file size - There may not be a partial record at the end. */

	FileSize.LowPart = GetFileSize (hInputFile, &FileSize.HighPart);
	nRecord = FileSize.QuadPart / RECORD_SIZE;
	if ((FileSize.QuadPart % RECORD_SIZE) != 0)
		nRecord++;

	/* Initiate a read on each buffer, corresponding
		to the overlapped structure. */

	CurPosIn.QuadPart = 0;
	for (ic = 0; ic < MAX_OVRLP; ic++) {
		OverLapIn [ic].hEvent = (HANDLE) ic;	/* Overload the event fields to */
		OverLapOut [ic].hEvent = (HANDLE) ic;	/* hold the event number. */

						/* Set file position. */
		OverLapIn [ic].Offset = CurPosIn.LowPart;
		OverLapIn [ic].OffsetHigh = CurPosIn.HighPart;
		if (CurPosIn.QuadPart < FileSize.QuadPart)
			ReadFileEx (hInputFile, AsRec [ic], RECORD_SIZE,
					&OverLapIn [ic], ReadDone);
		CurPosIn.QuadPart += (LONGLONG) RECORD_SIZE;
	}

	/*  All read operations are running. Enter an alterable
		wait state and continue until all records have been processed. */

	nDone = 0;
	while (nDone < 2* nRecord)
		SleepEx (0, TRUE);
	CloseHandle (hInputFile);
	CloseHandle (hOutputFile);
	_tprintf (_T ("ASCII to Unicode conversion completed.\n"));
	return 0;
}

static VOID WINAPI ReadDone (DWORD Code, DWORD nBytes, LPOVERLAPPED pOv)
{
	LARGE_INTEGER CurPosIn, CurPosOut;
	DWORD ic, i;

	nDone++;
		/* Process the record and initiate the write. */
		/* Get the overlapped structure ID from the event field. */
	ic = (DWORD) (pOv->hEvent);
	CurPosIn.LowPart = OverLapIn [ic].Offset;
	CurPosIn.HighPart = OverLapIn[ic].OffsetHigh;
	CurPosOut.QuadPart = (CurPosIn.QuadPart / RECORD_SIZE) * URECORD_SIZE;
	OverLapOut [ic].Offset = CurPosOut.LowPart;
	OverLapOut [ic].OffsetHigh = CurPosOut.HighPart;
			/* Convert an ASCII record to Unicode. */
	for (i = 0; i < nBytes; i++)
		UnRec [ic] [i] = AsRec [ic] [i];
	WriteFileEx (hOutputFile, UnRec [ic], nBytes * 2,
		&OverLapOut [ic], WriteDone);

	/* Prepare the input overlapped structure
		for the next read, which will be initiated
		after the write, issued above, completes. */

	CurPosIn.QuadPart += RECORD_SIZE * (LONGLONG) (MAX_OVRLP);
	OverLapIn [ic].Offset = CurPosIn.LowPart;
	OverLapIn [ic].OffsetHigh = CurPosIn.HighPart;

	return;
}

static VOID WINAPI WriteDone (DWORD Code, DWORD nBytes, LPOVERLAPPED pOv)
{
	LARGE_INTEGER CurPosIn;
	DWORD ic;
	nDone++;
			/* Get the overlapped structure ID from the event field. */
	ic = (DWORD) (pOv->hEvent);

	/* Start the read. The file position has already
		been set in the input overlapped structure.
		Check first, however, to assure that we
		do not read past the end of file. */

	CurPosIn.LowPart = OverLapIn [ic].Offset;
	CurPosIn.HighPart = OverLapIn [ic].OffsetHigh;
	if (CurPosIn.QuadPart < FileSize.QuadPart) {
			/* Start a new read. */
		ReadFileEx (hInputFile, AsRec [ic], RECORD_SIZE,
				&OverLapIn [ic], ReadDone);
	}
	return;
}
