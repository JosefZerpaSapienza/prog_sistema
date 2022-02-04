/* Chapter 6, sortMM command.
	Memory Mapped version - Minimal error reporting. */

/*  sortMM [options] [file]
	Sort one file. The extension to multiple files is straightforward but is omitted here.

	Options:
	-r	Sort in reverse order.
	-I	Use an existing index file to produce the sorted file.

	This limited implementation sorts on the first field only.
	The length of the Key field is determined by the length of the first
	white space terminated field of the first record (line). */

/* This program illustrates:
	1.	Mapping files to memory.
	2.	Use of memory-based string processing functions in a mapped file.
	3.	Use of a memory based function (qsort) with a memory-mapped file.
	4.	_based pointers. */

/* Technique:
	1.	Map the input file (the file to be sorted).
	2.	Using standard character processing, scan the input file,
		placing each key (which is fixed size) into the "index" file record
		(the input file with a .idx suffix).
		The index file is created using file I/O; later it will be mapped.
	3.	Also place the start position of the record in the index file record.
		This is a _based pointer to the mapped input file.
	4.	Map the index file.
	5.	Use the C library qsort to sort the index file
		(which has fixed length records consisting of key and _based pointer).
	6.	Scan the sorted index file, putting input file records in sorted order.
	7.	Skip steps 2, 3, and 5 if the -I option is specified.
		Notice that this requires _based pointers. */

#include "EvryThng.h"

int KeyCompare (LPCTSTR, LPCTSTR);
DWORD CreateIndexFile (DWORD, LPCTSTR, LPTSTR);
DWORD KStart, KSize;	/* Key start position & size (TCHAR). */
BOOL Revrs;

int _tmain (int argc, LPTSTR argv [])
{
	/* The file is the first argument. Sorting is done in place. */
	/* Sorting is done by file memory mapping. */

	HANDLE hInFile, hInMap;	/* Input file handles. */
	HANDLE hXFile, hXMap;	/* Index file handles. */
	HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);
	BOOL IdxExists, NoPrint;
	DWORD FsIn, FsX, RSize, iKey, nWrite, *pSizes;
	LPTSTR pInFile = NULL;
	LPBYTE pXFile = NULL, pX; 
	TCHAR _based (pInFile) *pIn;
	TCHAR IdxFlNam [MAX_PATH], ChNewLine = '\n';
	int FlIdx;

		/* Determine the options. */

 	FlIdx = Options (argc, argv, _T ("rIn"), &Revrs, &IdxExists, &NoPrint, NULL);
	if (FlIdx >= argc)
		ReportError (_T ("No file name on command line."), 1, FALSE);

/* Step 1: Open and Map the Input File. */

	hInFile = CreateFile (argv [FlIdx], GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, 0, NULL);
	if (hInFile == INVALID_HANDLE_VALUE)
		ReportError (_T ("Failed to open input file."), 2, TRUE);

		/* Create a file mapping object. Use the file size. */

	hInMap = CreateFileMapping (hInFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hInMap == NULL)
		ReportError (_T ("Failed to create input file mapping."), 3, TRUE);
	pInFile = MapViewOfFile (hInMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pInFile == NULL)
		ReportError (_T ("Failed to map input file."), 4, TRUE);

		/* Get the file size.
			As the mapping succeeded, the file size is less than 2 GB. */

	FsIn = GetFileSize (hInFile, NULL);

		/* Create the index file name. */

	_stprintf (IdxFlNam, _T ("%s%s"), argv [FlIdx], _T (".idx"));

/* Steps 2 and 3, if necessary. */

	if (!IdxExists)
		RSize = CreateIndexFile (FsIn, IdxFlNam, pInFile);

/* Step 4. Map the index file. */

	hXFile = CreateFile (IdxFlNam, GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, 0, NULL);
	if (hXFile == INVALID_HANDLE_VALUE)
		ReportError (_T ("Failed to open existing index file."), 5, TRUE);

		/* Create a file mapping object. Use the file size. */

	hXMap = CreateFileMapping (hXFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hXMap == NULL)
		ReportError (_T ("Failed to create index file mapping."), 6, TRUE);
	pXFile = MapViewOfFile (hXMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);	
	if (pXFile == NULL)
		ReportError (_T ("Failed to map index file."), 7, TRUE);
	FsX = GetFileSize (hXFile, NULL);

	/* Get the key size/key start and adjust the file size for the
		KeySize/KeyStart fields. Compute the record size from the key size. */

	pSizes = (LPDWORD)pXFile; KSize = *pSizes; 
	KStart = *(pSizes + 1);
	FsX -= 2 * sizeof (DWORD); 

/* Step 5. Sort the index file using qsort. */

	if (!IdxExists)
		qsort (pXFile + 2 * sizeof (DWORD), FsX / RSize, RSize, KeyCompare);

/* Step 6. Output the input file in sorted order. */

		/* Point to the address of the input file string.
			Start in the Input file. */

	pX = pXFile + 2 * sizeof (DWORD) + RSize - sizeof (LPTSTR);

	if (!NoPrint)
	for (iKey = 0; iKey < FsX / RSize; iKey++) {		
		WriteFile (hStdOut, &ChNewLine, TSIZE, &nWrite, NULL);

			/* The cast on pX is important, as it is a pointer to a
				byte and we need the four bytes of a based pointer. */
		pIn = (TCHAR _based (pInFile)*) *(LPDWORD) pX;
		
		while ((*pIn != CR || *(pIn + 1) != LF) && (DWORD) pIn < FsIn) {
			WriteFile (hStdOut, pIn, TSIZE, &nWrite, NULL);
			pIn++;
		}
		pX += RSize;
 	}

	/* Done. Free all the handles and maps. */

	UnmapViewOfFile (pInFile);
	CloseHandle (hInMap);
	CloseHandle (hInFile);
	UnmapViewOfFile (pXFile);
	CloseHandle (hXMap);
	CloseHandle (hXFile);
	return 0;
}
	
DWORD CreateIndexFile (DWORD FsIn, LPCTSTR IdxFlNam, LPTSTR pInFile)

/* Perform Steps 2-3 as defined in program description. */
/* This step will be skipped if the options specify use of an existing index file. */
{
	HANDLE hXFile;
	TCHAR _based (pInFile) *pInScan = 0;
	DWORD nWrite;

	/* Step 2a: Create an index file.
		Do not map it yet as its length is not known. */

	hXFile = CreateFile (IdxFlNam, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (hXFile == INVALID_HANDLE_VALUE)
		ReportError (_T ("Failure to create index file."), 8, TRUE);
	
	/* Step 2b: Get the first key and determine key size and start. */

	KStart = (DWORD) pInScan;
				/* Computed start of key field. */
	while (*pInScan != ' ' && *pInScan != '\t') pInScan++;
				/* Computed end of key field */

	KSize = ((DWORD) pInScan - KStart) / TSIZE;

		/* Computed key field size in characters. */

	/* Step 2c. Step 3. Scan the complete file, writing keys
		and record pointers to the key file. */
	/* The eight bytes contain the Key Size/Key Start.
		This is necessary so that we can re-use the index file. */

	WriteFile (hXFile, &KSize, sizeof (DWORD), &nWrite, NULL);
	WriteFile (hXFile, &KStart, sizeof (DWORD), &nWrite, NULL);
	pInScan = /*(TCHAR _based (pInFile)*)*/0;
	while ((DWORD) pInScan < FsIn) {
		WriteFile (hXFile, pInScan + KStart, KSize * TSIZE, &nWrite, NULL);
		WriteFile (hXFile, &pInScan, sizeof (LPTSTR), &nWrite, NULL);
		while ((DWORD) pInScan < FsIn && ((*pInScan != CR)
				|| (*(pInScan + 1) != LF))) {
			pInScan++; /* Skip to end of line. */
		}
		pInScan += 2; /* Skip past CR, LF. */
	}
	CloseHandle (hXFile);
			/* Size of an individual record. */
	return KSize * TSIZE + sizeof (LPTSTR);
}

int KeyCompare (LPCTSTR pKey1, LPCTSTR pKey2)

/* Compare two records of generic characters.
	The key position and length are global variables. */
{
	DWORD i;
	TCHAR t1, t2;
	int Result = 0;
	for (i = 0; i < KSize && Result == 0; i++) {
		t1 = *pKey1;
		t2 = *pKey2;
		if (t1 < t2)
			Result = -1;
		if (t1 > t2)
			Result = +1;
		pKey1++;
		pKey2++;
	}
	return Revrs ? -Result : Result;
}








