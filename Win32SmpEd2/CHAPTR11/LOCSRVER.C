/* Chapter 9. LocSrver.c */

/* Find a server by reading the mailslot that is used to broadcast server names. */
/* This function illustrates a mailslot client. */

#include "EvryThng.h"
		/* The mailslot name is defined in "ClntSrvr.h". */
#include "ClntSrvr.h"

BOOL LocateServer (LPTSTR pPipeName)
{
	HANDLE MsFile;
	MS_MESSAGE ServerMsg;
	BOOL Found = FALSE;
	DWORD cbRead;

	MsFile = CreateMailslot (MS_SRVNAME, 0, CS_TIMEOUT, NULL);
		if (MsFile == INVALID_HANDLE_VALUE)
			ReportError (_T ("MS create error."), 11, TRUE);

	/* Communicate with the server to be certain that it is running.
		The server must have time to find the mailslot and send the pipe name. */
	
	while (!Found) {
		_tprintf (_T ("Looking for a server.\n"));
		Found = ReadFile (MsFile, &ServerMsg, MSM_SIZE, &cbRead, NULL);
	}

	_tprintf (_T ("Server has been located.\n"));

	/* Close the mailslot. */

	CloseHandle (MsFile);
	_tcscpy (pPipeName, ServerMsg.msName);
	return TRUE;
}
