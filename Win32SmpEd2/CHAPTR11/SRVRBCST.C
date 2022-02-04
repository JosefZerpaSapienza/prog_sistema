/* Program 9-4. Mailslot client.
	Broadcast the pipe name to waiting command line clients
	using a mailslot. The mailslot is multiple readers
	(every client) and single (or multiple) writers (every server). */

#include "EvryThng.h"
#include "ClntSrvr.h"

VOID _tmain (int argc, LPTSTR argv [])
{
	BOOL exit;
	MS_MESSAGE MsNotify;
	DWORD nXfer;
	HANDLE hMsFile;

	/* Open the mailslot for the MS "client" writer. */

	while (TRUE) {
		exit = FALSE;
		while (!exit) { /* Wait for a client to create a MS. */
			hMsFile = CreateFile (MS_CLTNAME, GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hMsFile == INVALID_HANDLE_VALUE) {
				Sleep (5000);
			}
			else exit = TRUE;
	  }
		/* Send out the message to the mailslot. */

		MsNotify.msStatus = 0;
		MsNotify.msUtilization = 0;
		_tcscpy (MsNotify.msName, SERVER_PIPE);
		if (!WriteFile (hMsFile, &MsNotify, MSM_SIZE, &nXfer, NULL))
			ReportError (_T ("Server MS Write error."), 13, TRUE);
		CloseHandle (hMsFile);

		/* Wait for another client to open a mailslot. */
	}

	/* Not reachable. */
	return;
}
