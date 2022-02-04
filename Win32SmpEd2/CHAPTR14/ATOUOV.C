/* Chapter 14. atouOV.
	OVERLAPPED I/O ASCII to Unicode file conversion. 
	Windows 2000 and NT only */
/* atouOV file1 file2 */
/* This program illustrates overlapped asynch I/O. */

#include "EvryThng.h"

#define MAX_OVRLP 4
#define REC_SIZE 0x8000 /* 32K is the minimum size to get decent performance */
		/* However, even the best performance is about half that of atouMM */
#define UREC_SIZE 2 * REC_SIZE

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInputFile, hOutputFile;
	/* There is a copy of each of the followinng variables and */
	/* structures for each outstanding overlapped I/O operation */
	DWORD nin [MAX_OVRLP], nout [MAX_OVRLP], ic, i;
	OVERLAPPED OverLapIn [MAX_OVRLP], OverLapOut [MAX_OVRLP];
	/* The first event index is 0 for read, 1 for write */
	/* WaitForMultipleObjects requires a contiguous array */
	HANDLE hEvents [2] [MAX_OVRLP];
	/* The first index on these two buffers is the I/O operation */
	CHAR AsRec [MAX_OVRLP] [REC_SIZE];
	WCHAR UnRec [MAX_OVRLP] [REC_SIZE];
	LARGE_INTEGER CurPosIn, CurPosOut, FileSize;
	LONGLONG nRecord, iWaits;

	if (argc != 3)
		ReportError (_T ("Usage: atouOV file1 file2"), 1, FALSE);

	/* Note: The "no buffering" flags are turned off because they require the file length
	to be a sector size mutliple, reducing the program's generality. However, when the file
	size does meet this condition, you will get a slight performance boost (about 15% in
	some experiments */
	hInputFile = CreateFile (argv [1], GENERIC_READ,
			0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED /*| FILE_FLAG_NO_BUFFERING*/, NULL);
	if (hInputFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening input file."), 2, TRUE);
	
	hOutputFile = CreateFile (argv [2], GENERIC_WRITE,
			0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED /*| FILE_FLAG_NO_BUFFERING*/, NULL);
	if (hOutputFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening output file."), 3, TRUE);

	/* Compute the total number of records to be processed
		from the input file size - There may not be a partial
		record at the end. */

	FileSize.LowPart = GetFileSize (hInputFile, &FileSize.HighPart);
	nRecord = FileSize.QuadPart / REC_SIZE;
	if ((FileSize.QuadPart % REC_SIZE) != 0) nRecord++;

	/*  Create the manual-reset, unsignaled events for 
		the overlapped structures. Initiate a read on
		each buffer, corresponding to the overlapped structure. */

	CurPosIn.QuadPart = 0;
	for (ic = 0; ic < MAX_OVRLP; ic++) {
		hEvents [0] [ic] = OverLapIn [ic].hEvent
					/* Input complete event. */
			= CreateEvent (NULL, TRUE, FALSE, NULL); 
		hEvents [1] [ic] = OverLapOut [ic].hEvent
					/* Output complete event. */
			= CreateEvent (NULL, TRUE, FALSE, NULL); 
					/* Set file position. */
		OverLapIn [ic].Offset = CurPosIn.LowPart;
		OverLapIn [ic].OffsetHigh = CurPosIn.HighPart;
		if (CurPosIn.QuadPart < FileSize.QuadPart)
			ReadFile (hInputFile, AsRec [ic], REC_SIZE,
					&nin [ic], &OverLapIn [ic]);
		CurPosIn.QuadPart += (LONGLONG) REC_SIZE;
	}

	/*  All read operations are running. Wait for an
		event to complete and reset it immediately.
		We wait for both read and write events.
		Continue until all records have been processed. */

	iWaits = 0;
	while (iWaits < 2 * nRecord) {
		ic = WaitForMultipleObjects (2 * MAX_OVRLP,
				hEvents [0], FALSE, INFINITE) - WAIT_OBJECT_0;
		iWaits++;
		ResetEvent (hEvents [ic / MAX_OVRLP] [ic % MAX_OVRLP]);

		if (ic < MAX_OVRLP) { /* A read completed. */
			if (!GetOverlappedResult (hInputFile,
					&OverLapIn [ic], &nin [ic], FALSE))
				ReportError (_T ("Read failed."), 0, TRUE);

			/* Process the record and initiate the write. */

			CurPosIn.LowPart = OverLapIn [ic].Offset;
			CurPosIn.HighPart = OverLapIn [ic].OffsetHigh;
			CurPosOut.QuadPart = 
				(CurPosIn.QuadPart / REC_SIZE) * UREC_SIZE;
			OverLapOut [ic].Offset = CurPosOut.LowPart;
			OverLapOut [ic].OffsetHigh = CurPosOut.HighPart;
			
			/* Convert an ASCII record to Unicode. */

			for (i = 0; i < REC_SIZE; i++)
				UnRec [ic] [i] = AsRec [ic] [i];
			WriteFile (hOutputFile, UnRec [ic], nin [ic] * 2,
					&nout [ic], &OverLapOut [ic]);

			/* Prepare the input overlapped structure
				for the next read, which will be initiated
				after the write, issued above, completes. */

			CurPosIn.QuadPart += 
					REC_SIZE * (LONGLONG) (MAX_OVRLP);
			OverLapIn [ic].Offset = CurPosIn.LowPart;
			OverLapIn [ic].OffsetHigh = CurPosIn.HighPart;

		} else if (ic < 2 * MAX_OVRLP) {	/* A write completed. */

			/* Start the read. The file position has already
				been set in the input overlapped structure.
				Check first, however, to assure that we
				do not read past the end of file. */

			ic -= MAX_OVRLP;	/* Set the output buffer index. */
			if (!GetOverlappedResult (hOutputFile,
					&OverLapOut [ic], &nout [ic], FALSE))
				ReportError (_T ("Read failed."), 0, TRUE);

			CurPosIn.LowPart = OverLapIn [ic].Offset;
			CurPosIn.HighPart = OverLapIn [ic].OffsetHigh;
			if (CurPosIn.QuadPart < FileSize.QuadPart) {
						/* Start a new read. */
				ReadFile (hInputFile, AsRec [ic], REC_SIZE,
						&nin [ic], &OverLapIn [ic]);
			}
		}
		else	/* Impossible unless wait failed error. */

			ReportError (_T ("Multiple wait error."), 0, TRUE);
	}
				/*  Close all events. */
	for (ic = 0; ic < MAX_OVRLP; ic++) {
		CloseHandle (hEvents [0] [ic]);
		CloseHandle (hEvents [1] [ic]);
	}
	CloseHandle (hInputFile);
	CloseHandle (hOutputFile);
	_tprintf (_T ("ASCII to Unicode conversion completed.\n"));
	return 0;
}
