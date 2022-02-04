/* Create a file of randomly keyed records.
	Each record is 64 bytes, keyed by the first 8. */
/*  random nRec outfile	*/

#include "EvryThng.h"

static VOID srandx (DWORD);	/* MS version RAND_MAX is only */
static DWORD randx (void);	/* 32K - too small. Do our own. */

int _tmain (int argc, LPTSTR argv [])
{
	DWORD nRec, i;
	FILE *fp;
	DWORD iR;
	FILETIME FileTime;	/* Times to seed random #s. */
	SYSTEMTIME SysTi;
	TCHAR Buffer [256];

	nRec = _ttoi (argv [1]);
	fp = _tfopen (argv [2], _T("w"));
	if (fp == NULL)
		ReportError (_T ("Failed opening output."), 1, TRUE);
	GetSystemTime (&SysTi);
	SystemTimeToFileTime (&SysTi, &FileTime);

	srandx (FileTime.dwLowDateTime);
	for (i = 0; i < nRec; i++) {
		iR = randx ();
		_stprintf (Buffer, _T("%08x. Record Number: %08d.abcdefghijklmnopqrstuvwxyz x\n"), iR, i);
		printf ("length of buffer %d %d\n", _tcslen (Buffer), sizeof(TCHAR));
		fwrite (Buffer, _tcslen(Buffer)+1, sizeof(TCHAR), fp);
	}

	fclose (fp);
	return 0;
}

static DWORD next = 1;	/* Adapted from Plauger, p. 337. */

DWORD randx (VOID)	/* Cycle length was tested to be 2**32. */
{
	next = next * 1103515245 + 12345;
	return next;
}

VOID srandx (DWORD seed)
{
	next = seed;
	return;
}
