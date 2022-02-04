/* Chapter 4. CNTRLC.C */
/* Catch cntrl-c signals. */

#include "EvryThng.h"

static BOOL WINAPI Handler (DWORD CntrlEvent);
volatile static BOOL Exit = FALSE;

int _tmain (int argc, LPTSTR argv [])

/* Beep periodically until signaled to stop. */
{
		/* Add an event handler. */
	if (!SetConsoleCtrlHandler (Handler, TRUE))
		ReportError (_T ("Error setting event handler"), 1, TRUE);
	
	while (!Exit) { /* This flag is detected right after a beep, before a handler exits */
		Sleep (4750); /* Beep every 5 seconds; allowing 250 ms of beep time. */
		Beep (1000 /* Frequency */, 250 /* Duration */);
	}
	_tprintf (_T ("Stopping the main program as requested.\n"));
	return 0;
}	

BOOL WINAPI Handler (DWORD CntrlEvent)
{
	Exit = TRUE;

	switch (CntrlEvent) { 
		/* The signal timing will determine if you see the second handler message */
		case CTRL_C_EVENT:
			_tprintf (_T ("Ctrl-c received by handler. Leaving in 10 seconds\n"));
			Sleep (4000); /* Decrease this time to get a different effect */
			_tprintf (_T ("Leaving handler in 6 seconds.\n"));
			Sleep (6000);  /* Also try decreasing this time */
			return TRUE; /* TRUE indicates that the signal was handled. */
		case CTRL_CLOSE_EVENT:
			_tprintf (_T ("Leaving the handler in 10 seconds.\n"));
			Sleep (4000); /* Decrease this time to get a different effect */
			_tprintf (_T ("Leaving handler in 6 seconds.\n"));
			Sleep (6000);  /* Also try decreasing this time */
			return TRUE; /* Try returning FALSE. Any difference? */
		default:
			_tprintf (_T ("Event: %d. Leaving in 10 seconds\n"), CntrlEvent);
			Sleep (4000); /* Decrease this time to get a different effect */
			_tprintf (_T ("Leaving handler in 6 seconds.\n"));
			Sleep (6000);  /* Also try decreasing this time */
			return TRUE; /* TRUE indicates that the signal was handled. */
	}
}
