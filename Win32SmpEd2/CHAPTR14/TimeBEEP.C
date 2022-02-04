/* Chapter 14 -  TimeBeep.c. Periodic alarm. */
/* Usage: TimeBeep Period (in ms). */
/* This implementation uses the "waitable timers" introduced
   with NT 4.0 and supported first in Visual C++ Ver 5.0. 
   * * * Not available on Windows 9x and Windows CE. * * * *
   The waitable timers are sharable
   named objects (their names share the same space used by
   synchronization and memory mapped objects. 
   The waitable timer is a synchronization object which you
   can wait on for the timer to signal. Alternatively, you can
   have a completion routine executed in the same thread by
   entering an "alertable wait state" just as with extended,
   ("completion routine") I/O. 
   This program uses a console control handler to catch
   control C signals; this subject is covered at the end of
   Chapter 7.  */

#define _WIN32_WINNT 0x0400	/*  Required in <winbase.h> to define
		waitable timer functions. This is not explained in the
		the documentation.  */
#include "EvryThng.h"

static BOOL WINAPI Handler (DWORD CntrlEvent);
static VOID APIENTRY Beeper (LPVOID, DWORD, DWORD);
volatile static BOOL Exit = FALSE;

HANDLE hTimer;

int _tmain (int argc, LPTSTR argv [])
{
	DWORD Count = 0, Period;
	LARGE_INTEGER DueTime;

	if (argc >= 2)
		Period = _ttoi (argv [1]) * 1000;
	else ReportError (_T ("Usage: TimeBeep period(in seconds)"), 1, FALSE);

	if (!SetConsoleCtrlHandler (Handler, TRUE))
		ReportError (_T ("Error setting event handler"), 2, TRUE);

	DueTime.QuadPart = -(LONGLONG)Period * 10000;
			/*  Due time is negative for first time out relative to
				current time. Period is in ms (10**-3 sec) whereas
				the due time is in 100 ns (10**-7 sec) units to be
				consistent with a FILETIME. */

	hTimer = CreateWaitableTimer (NULL /* Security attributes */,
		FALSE /*TRUE*/,	/*  Not manual reset (a "notification timer") but
						a "synchronization timer." Documentation does not
						explain the distinction clearly, but the ST is
						associated with a callback function. */
		NULL		/*  Do not name the timer - name space is shared with
						events, mutexes, semaphores, and mem map objects */);
	
	if (hTimer == NULL) 
		ReportError (_T ("Failure creating waitable timer"), 3, TRUE);
	if (!SetWaitableTimer (hTimer, 
			&DueTime /* relative time of first signal. Positive value would
						indicate an absolute time. */,
			Period  /* Time period in ms */,
			Beeper  /* Timer function */,
			&Count  /* Parameter passed to timer function */,
			TRUE    /* Does not apply - do not use it. */))
		ReportError (_T ("Failure setting waitable timer"), 4, TRUE);

	/*	Enter the main loop */
	while (!Exit) {
		_tprintf (_T("Count = %d\n"), Count); 
		/* Count is increased in the timer routine */
		/*  Enter an alertable wait state, enabling the timer routine.
			The timer handle is a synchronization object, so you can 
			also wait on it. */
		SleepEx (INFINITE, TRUE);
//		WaitForSingleObject (hTimer, INFINITE);

		/* Note the different results if you use the wait instead of SleepEx. Explanation?? */
	}

	_tprintf (_T("Shut down. Count = %d"), Count);
	CancelWaitableTimer (hTimer);
	CloseHandle (hTimer);
	return 0;
}	

static VOID APIENTRY Beeper (LPVOID lpCount,
   DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{

	*(LPDWORD)lpCount = *(LPDWORD)lpCount + 1;

	_tprintf (_T("About to perform beep number: %d\n"), *(LPDWORD)lpCount);
	Beep (1000 /* Frequency */, 250 /* Duration (ms) */);
	return;
}

BOOL WINAPI Handler (DWORD CntrlEvent)
{
	Exit = TRUE;
	_tprintf (_T("Shutting Down\n"));
	CloseHandle (hTimer);
	return TRUE;
}
