/* Chapter 3. rm file delete command. */
/* rm [options] files */
/* This program is similar to ls (Program 3-2) with the 
	significant changes in thr ProcessItem function */
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

	FileIndex = Options (argc, argv, _T ("ri"), &Flags [0], &Flags [1], NULL);

	GetCurrentDirectory (MAX_PATH, CurrPath);
	if (argc < FileIndex + 1) 
		ok = TraverseDirectory (_T ("*"), MAX_OPTIONS, Flags);
	else for (i = FileIndex; i < argc; i++) {
		_tcscpy (PathName, argv [i]);

			/* Find the rightmost slash, if any.
				Set the path and use the rest as the file name. */

		pSlash = _tstrrchr (PathName, '\\'); 
		if (pSlash != NULL) {
			*pSlash = '\0';
			SetCurrentDirectory (PathName); /* Now restore pathname. */
			*pSlash = '\\';
			pFileName = pSlash + 1;
		} else pFileName = PathName;

		ok = TraverseDirectory (pFileName, MAX_OPTIONS, Flags) && ok;
		SetCurrentDirectory (CurrPath);
	}
	return ok ? 0 : 1;
}

static BOOL TraverseDirectory (LPCTSTR PathName, DWORD NumFlags, LPBOOL Flags)

/*	Traverse a directory - Carrying out an implementation-specific
	"action" for every name encountered. The action in this version is "Delete". */

/*	PathName: Relative or absolute pathname to traverse. */
{
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	BOOL Recursive = Flags [0];
	DWORD FType, iPass;

	/*	Open up the directory search handle and get the
		first file name to satisfy the pathname.
		Make two passes. The first processes the files
		and the second processes the directories. */

	for (iPass = 1; iPass <= 2; iPass++) {
		SearchHandle = FindFirstFile (PathName, &FindData);
		if (SearchHandle == INVALID_HANDLE_VALUE)
			/* A deleted file will not be found on the second pass */
			return iPass == 2; 

		/* Scan the directory and its subdirectories
			for files satisfying the pattern. */

		do {

		/* For each file located, get the type. Delete files on pass 1.
			On pass 2, recursively process the subdirectory
			contents, if the recursive option is set. */

			FType = FileType (&FindData);
			if (FType == TYPE_FILE && iPass == 1) /* Delete a file. */
				ProcessItem (&FindData, MAX_OPTIONS, Flags);

			/* Traverse the subdirectory on the second pass. */

			if (FType == TYPE_DIR && iPass == 2 && Recursive) {
				SetCurrentDirectory (FindData.cFileName);
				TraverseDirectory (_T ("*"), NumFlags, Flags);
				SetCurrentDirectory (_T (".."));
				ProcessItem (&FindData, MAX_OPTIONS, Flags);
			}

			/* Get the next file or directory name. */
		}
		while (FindNextFile (SearchHandle, &FindData));
		FindClose (SearchHandle);
	}
	return TRUE;
}

static BOOL ProcessItem (LPWIN32_FIND_DATA pFileData, DWORD NumFlags, LPBOOL Flags)

/* Function to process (delete, in this case) the file or directory.
	Interact with the user if the interact flag is set.
/* ParName is the parent directory name, terminated with
	a \ or the null string. FileName has no wildcards at this point. */
{
	DWORD FType;
	BOOL Interact = Flags [1], Proceed = TRUE;
	TCHAR YNResp [3];

	FType = FileType (pFileData);
	if (FType != TYPE_FILE && FType != TYPE_DIR) return FALSE;

	if (Interact) {
		_tprintf (_T ("\nDelete %s? [y/n]"), pFileData->cFileName);
		_tscanf (_T ("%s"), &YNResp);
		Proceed = (_totlower (YNResp [0]) == 'y');
	}
	
	if (!Proceed) return FALSE;
	if (FType == TYPE_FILE && !DeleteFile (pFileData->cFileName))
		ReportError (_T ("File Delete Failed"), 0, TRUE);
	if (FType == TYPE_DIR && !RemoveDirectory (pFileData->cFileName))
		ReportError (_T ("Directory Delete Failed"), 0, TRUE);
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

