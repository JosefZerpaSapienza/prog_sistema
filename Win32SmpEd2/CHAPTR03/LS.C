/* Chapter 3. ls file list command. */
/* ls [options] [files]
	List the attributes of one or more files.
	Options:
		-R	Recursive
		-l	Long listing (size and time of modification)
			Depending on the ProcessItem function, this will
			also list the owner and permissions (See Chapter 5 - security). */

/* This program illustrates:
		1.	Search handles and directory traversal
		2.	File attributes, including time
		3.	Using generic string functions to output file information */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* BEGIN BOILERPLATE CODE */

#include "EvryThng.h"

BOOL TraverseDirectory (LPCTSTR, DWORD, LPBOOL);
DWORD FileType (LPWIN32_FIND_DATA);
BOOL ProcessItem (LPWIN32_FIND_DATA, DWORD, LPBOOL);

int _tmain (int argc, LPTSTR argv [])
{
	BOOL Flags [MAX_OPTIONS], ok = TRUE;
	TCHAR PathName [MAX_PATH + 1], CurrPath [MAX_PATH + 1];
	LPTSTR pSlash, pFileName;
	int i, FileIndex;

	FileIndex = Options (argc, argv, _T ("Rl"), &Flags [0], &Flags [1], NULL);

	/* "Parse" the search pattern into two parts: the "parent"
		and the file name or wild card expression. The file name
		is the longest suffix not containing a slash.
		The parent is the remaining prefix with the slash.
		This is performed for all command line search patterns.
		If no file is specified, use * as the search pattern. */

	GetCurrentDirectory (MAX_PATH, CurrPath); /* Save. */
	if (argc < FileIndex + 1) 
		ok = TraverseDirectory (_T ("*"), MAX_OPTIONS, Flags);
	else for (i = FileIndex; i < argc; i++) {
		_tcscpy (PathName, argv [i]);

		/* Find the rightmost slash, if any.
			Set the path and use the rest as the file name. */

		pSlash = _tstrrchr (PathName, '\\'); 

		if (pSlash != NULL) {
			*pSlash = '\0';
			SetCurrentDirectory (PathName); /* Now restore pathname.*/
			*pSlash = '\\'; pFileName = pSlash + 1;
		} else pFileName = PathName;
		ok = TraverseDirectory (pFileName, MAX_OPTIONS, Flags) && ok;
		SetCurrentDirectory (CurrPath);	 /* Restore working directory. */
	}

	return ok ? 0 : 1;
}

static BOOL TraverseDirectory (LPCTSTR PathName, DWORD NumFlags, LPBOOL Flags)

/* Traverse a directory, carrying out an implementation specific "action" for every
	name encountered. The action in this version is "list, with optional attributes". */

/* PathName: Relative or absolute pathname to traverse.  */
{
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	BOOL Recursive = Flags [0];
	DWORD FType, iPass;
	TCHAR CurrPath [MAX_PATH + 1];

	/* Open up the directory search handle and get the
		first file name to satisfy the path name.
		Make two passes. The first processes the files
		and the second processes the directories. */

	GetCurrentDirectory (MAX_PATH, CurrPath);

	for (iPass = 1; iPass <= 2; iPass++) {
		SearchHandle = FindFirstFile (PathName, &FindData);
		if (SearchHandle == INVALID_HANDLE_VALUE) {
			ReportError (_T ("Error opening Search Handle."), 0, TRUE);
			return FALSE;
		}
		/* Scan the directory and its subdirectories
			for files satisfying the pattern. */

		do {

		/* For each file located, get the type. List everything on pass 1.
			On pass 2, display the directory name and recursively process
			the subdirectory contents, if the recursive option is set. */

			FType = FileType (&FindData);
			if (iPass == 1) /* ProcessItem is "print attributes". */
				ProcessItem (&FindData, MAX_OPTIONS, Flags);

			/* Traverse the subdirectory on the second pass. */

			if (FType == TYPE_DIR && iPass == 2 && Recursive) {
				_tprintf (_T ("\n%s\\%s:"), CurrPath, FindData.cFileName);
				SetCurrentDirectory (FindData.cFileName);
				TraverseDirectory (_T ("*"), NumFlags, Flags);
				SetCurrentDirectory (_T (".."));
			}

			/* Get the next file or directory name. */

		} while (FindNextFile (SearchHandle, &FindData));

		FindClose (SearchHandle);
	}
	return TRUE;
}

static DWORD FileType (LPWIN32_FIND_DATA pFileData)

/* Return file type from the find data structure.
	Types supported:
		TYPE_FILE:	If this is a file
		TYPE_DIR:	If this is a directory other than . or ..
		TYPE_DOT:	If this is . or .. directory */
{
	BOOL IsDir;
	DWORD FType;
	FType = TYPE_FILE;
	IsDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (IsDir)
		if (lstrcmp (pFileData->cFileName, _T (".")) == 0
				|| lstrcmp (pFileData->cFileName, _T ("..")) == 0)
			FType = TYPE_DOT;
		else FType = TYPE_DIR;
	return FType;
}
/*  END OF BOILERPLATE CODE */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static BOOL ProcessItem (LPWIN32_FIND_DATA pFileData, DWORD NumFlags, LPBOOL Flags)

/* Function to process (list attributes, in this case)
	the file or directory. This implementation only shows
	the low order part of the file size. */
{
	const TCHAR FileTypeChar [] = {' ', 'd'};
	DWORD FType = FileType (pFileData);
	BOOL Long = Flags [1];
	SYSTEMTIME LastWrite;

	if (FType != TYPE_FILE && FType != TYPE_DIR) return FALSE;

	_tprintf (_T ("\n"));
	if (Long) {
		_tprintf (_T ("%c"), FileTypeChar [FType - 1]);
		_tprintf (_T ("%10d"), pFileData->nFileSizeLow);
		FileTimeToSystemTime (&(pFileData->ftLastWriteTime), &LastWrite);
		_tprintf (_T ("	%02d/%02d/%04d %02d:%02d:%02d"),
				LastWrite.wMonth, LastWrite.wDay,
				LastWrite.wYear, LastWrite.wHour,
				LastWrite.wMinute, LastWrite.wSecond);
	}
	_tprintf (_T (" %s"), pFileData->cFileName);
	return TRUE;
}
