/* Chapter 3. touch command. */
/* touch [options] files
	Update the access and modification times of the files.
	Create non-existent files.
	Options:
		-a Only update the access time.
		-m Only update the modification (last write) time.
		-c Do not create a non-existent file. */

/* This program illustrates:
	1. Setting the file time attributes.
	2. Converting from system to file time. */

#include "EvryThng.h"

int _tmain (int argc, LPTSTR argv [])
{
	SYSTEMTIME SysTime;
	FILETIME NewFileTime;
	LPFILETIME pAccessTime = NULL, pModifyTime = NULL;
	HANDLE hFile;
	BOOL Flags [MAX_OPTIONS];
	BOOL SetAccessTime, SetModTime, CreateNew;
	DWORD CreateFlag;
	int i, FileIndex;

	/* Determine the options. */

	if (argc < 2) {
		_tprintf (_T("Usage: touch [options] files"));
		return 1;
	}
	FileIndex = Options (argc, argv, _T ("amc"),
			&Flags [0], &Flags [1], &Flags [2], NULL);

	SetAccessTime = !Flags [0];
	SetModTime = !Flags [1];
	CreateNew = !Flags [2];
	CreateFlag = CreateNew ? OPEN_ALWAYS : OPEN_EXISTING;

	for (i = FileIndex; i < argc; i++) {
		hFile = CreateFile (argv [i], GENERIC_READ | GENERIC_WRITE, 0, NULL,
				CreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			ReportError (_T ("touch error: Cannot open file."), 1, TRUE);

		/* Get current system time and convert to a file time.
			Do not change the create time. */

		GetSystemTime (&SysTime);
		SystemTimeToFileTime (&SysTime, &NewFileTime);
		if (SetAccessTime) pAccessTime = &NewFileTime;
		if (SetModTime) pModifyTime = &NewFileTime;
		if (!SetFileTime (hFile, NULL, pAccessTime, pModifyTime))
			ReportError (_T ("Failure setting file times."), 2, TRUE);
		CloseHandle (hFile);
	}
	return 0;
}
