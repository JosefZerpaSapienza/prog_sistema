/*  Test Locking. Chapter 3. */
#include "EvryThng.h"

/*  Usage: TestLock FileName mode
Mode: 1 - Get a share lock and hold it until prompted to release it 
      2 - Get an exclusive lock and hold it until prompted to release
      3 - Attempt to read the file, then retry when prompted
	  4 - Attempt to write to the file; retry when prompted  */

/*	This very simple program was developed to illustrate the
	file locking logic in Tables 7-1 and 7-2.
	You should have two console prompts and execute TestLock.exe
	in each using different "mode" values. First, create a small
	file to use in the test; assume its name is "lock.txt"
	For example, in conosle 1 execute:
		TestLock lock.txt 1
	to create a share lock. In the console 2 try in succession:
		TestLock lock.txt 2     You can not create an exclusive lock over a share lock
		TestLock lock.txt 1     You can create a second share lock
		TestLock lock.txt 4     You can not write over a share lock
		TestLock lock.txt 3     You can read over a share lock even without a lock.
*/

int main (int argc, LPSTR argv[])

{
	HANDLE fh;
	CHAR String[] = "Hello, world.", Buffer[8] = "12345678", c;
	int command;
	OVERLAPPED ov = {0, 0, 0, 0, NULL};
	DWORD nRead = 0, nWrite = 0;


	if (argc < 3) ReportError ("No file name and command", 1, FALSE);

	command = atoi (argv[2]);

	fh = CreateFile (argv[1], GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		0, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		ReportError ("Cannot open file", 2, TRUE);

	switch (command) {
		case 1:	/* Get a share lock and hold until prompted to release */
				if (!LockFileEx (fh, LOCKFILE_FAIL_IMMEDIATELY, 
					0, 32, 0, &ov))
					ReportError ("Share Lock Failed", 3, TRUE);
				printf ("\nType any character to release the share lock");
				scanf ("%c", &c);
				if (!UnlockFileEx (fh, 0, 32, 0, &ov))
					ReportError ("Share Unlock Failed", 3, TRUE);
				printf ("\nLock released");

				break;
		
		case 2:	/* Get an exclusive lock and hold until prompted to release */
				if (!LockFileEx (fh, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
					0, 32, 0, &ov))
					ReportError ("Exclusive Lock Failed", 3, TRUE);
				printf ("\nType any character to release the exclusive lock");
				scanf ("%c", &c);
				if (!UnlockFileEx (fh, 0, 32, 0, &ov))
					ReportError ("Exclusive Unlock Failed", 3, TRUE);
				printf ("\nLock released");

				break;

		case 3:	/* Read a record */
				if (!ReadFile (fh, Buffer, sizeof(Buffer), &nRead, NULL))
					ReportError ("File Read failed", 0, TRUE);
				else printf ("\nRead succeeded: %c %c", Buffer[0], Buffer[1]);
				break;

		case 4: /* Write a record */
				if (!WriteFile (fh, String, strlen(String) + 1, &nWrite, NULL))
						ReportError ("File Write failed", 0, TRUE);
				else printf ("\nWrite succeeded");
				break;
				
		default:
				break;
	}

	return 0;
}
