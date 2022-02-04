/* Chapter 13. atouMT_DH.
	Multithreaded ASCII to Unicode file conversion.
	Use threads as an alternative to overlapped I/O. */
/* atouMT file1 file2 */

#include "EvryThng.h"

#define MAX_OVRLP 8
#define REC_SIZE 0x2000
#define UREC_SIZE 2 * REC_SIZE

static DWORD WINAPI ReadWrite (DWORD);
static HANDLE hIn [MAX_OVRLP], hOut [MAX_OVRLP];

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInFile, hOutFile, hThr [MAX_OVRLP];
	HANDLE hProc = GetCurrentProcess ();
	DWORD i, ThId;

	if (argc != 3)
		ReportError (_T ("Usage: atouMT file1 file2"), 1, FALSE);
	hInFile = CreateFile (argv [1], GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
 	if (hInFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening input file."), 2, TRUE);
	hOutFile = CreateFile (argv [2], GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOutFile == INVALID_HANDLE_VALUE) 
		ReportError (_T ("Fatal error opening output file."), 3, TRUE);
		
	/* Duplicate input and output handles for each thread
		and create the thread. The only thread argument is
		its number (0, 1, 2, ..., MAX_OVRLP-1). The thread
		gets everything else it needs from global storage. */
	/* Note: This DuoplicateHandle version has problems. 
		It appears is if ReadFile and WriteFile are
		not thread safe when dealing with duplcated handles. This is the case
		even if you protect the ReadFile and WriteFile with a CRITIAL_SECTION
		and even if you use the no buffering and write through flags. 
		You will see errors if
		you compare the unicode output file with one generated with a correct
		atouXX program.
		A correct version, atouMT.c, is in this directory, and it uses
		CreateFile */

	for (i = 0; i < MAX_OVRLP; i++) {
		if (!DuplicateHandle (hProc, hInFile, hProc, &hIn [i],
				0, FALSE, DUPLICATE_SAME_ACCESS))
			ReportError (_T ("Fatal error duplicating input handle."), 4, TRUE);
		if (!DuplicateHandle (hProc, hOutFile, hProc, &hOut [i],
				0, FALSE, DUPLICATE_SAME_ACCESS))
			ReportError (_T ("Fatal error duplicating output handle."), 5, TRUE);
		hThr [i] = (HANDLE) _beginthreadex (NULL, 0, ReadWrite, (LPLONG) i, 0, &ThId);
		if (hThr [i] == NULL)
			ReportError (_T ("Error creating thread."), 6, TRUE);
	}

	WaitForMultipleObjects (MAX_OVRLP, hThr, TRUE, INFINITE);
	for (i = 0; i < MAX_OVRLP; i++)
		CloseHandle (hThr [i]);
	_tprintf (_T ("ASCII to Unicode conversion completed.\n"));
	return 0;
}

static DWORD WINAPI ReadWrite (DWORD iTh)
{
  	CHAR AsRec [REC_SIZE];
	WCHAR UnRec [REC_SIZE];
	BOOL Exit = FALSE;
	LARGE_INTEGER CurPosIn, CurPosOut;
	DWORD i, nRead = 1, nWrite;

	CurPosIn.QuadPart = (LONGLONG) REC_SIZE * iTh;
	CurPosOut.QuadPart = (LONGLONG) REC_SIZE * iTh * 2;
	while (nRead > 0) {
		SetFilePointer (hIn [iTh], CurPosIn.LowPart,
			&CurPosIn.HighPart, FILE_BEGIN);
		SetFilePointer (hOut [iTh], CurPosOut.LowPart,
			&CurPosOut.HighPart, FILE_BEGIN);
		ReadFile (hIn [iTh], AsRec, REC_SIZE, &nRead, NULL);
		for (i = 0; i < nRead; i++)
			UnRec [i] = AsRec [i];
		WriteFile (hOut [iTh], UnRec, nRead * 2, &nWrite, NULL);
		CurPosIn.QuadPart += (LONGLONG) REC_SIZE * MAX_OVRLP;
		CurPosOut.QuadPart += (LONGLONG) REC_SIZE * 2 * MAX_OVRLP;
	}

	CloseHandle (hIn [i]);
	CloseHandle (hOut [i]);
	_endthreadex (0);
	return 0;
}