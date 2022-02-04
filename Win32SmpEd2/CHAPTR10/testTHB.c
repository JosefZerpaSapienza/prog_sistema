/* Chapter 10. testTHB.c	  			*/
/* Test the threshold barrier extended object type		*/

#include "EvryThng.h"
#include "synchobj.h"
#include <time.h>
#define DELAY_COUNT 100
#define MAX_THREADS 1024

/*  Usage: test_thb nthread barrier_threshold			*/
/* start up nthread worker threads, each of which waits at	*/
/* the barrier for enough threads to arrive.			*/
/* The threads then all proceed. The threads log their time	*/
/* of arrival and departure from the barrier			*/

DWORD WINAPI worker (PVOID);

typedef struct _THARG {
	int thread_number;
	THRESHOLD_BARRIER *pbarrier;
	char waste [8]; /* Assure there is no quadword overlap */
} THARG;

int main (int argc, char * argv[])
{
	int tstatus, nthread, ithread, bvalue;
	HANDLE *worker_t;
	THB_HANDLE barrier;
	THARG * thread_arg;
	
	if (argc != 3) {
		_tprintf (_T("Usage: test_thb nthread barrier_threshold\n"));
		return 1;
	}
		
	srand ((DWORD)time(NULL)); /* Seed the random # generator 	*/

	nthread = atoi(argv[1]);
	if (nthread > MAX_THREADS) {
		_tprintf (_T("Maximum number of threads is %d.\n"), MAX_THREADS);
		return 2;
	}
	bvalue = atoi(argv[2]);
	worker_t = malloc (nthread * sizeof(HANDLE));
	thread_arg = calloc (nthread, sizeof (THARG));
	if (worker_t == NULL || thread_arg == NULL)
		ReportError (_T("Cannot allocate working memory"), 1, TRUE);
	
	tstatus = CreateThresholdBarrier (&barrier, bvalue);
	
	for (ithread = 0; ithread < nthread; ithread++) {
		/* Fillin the thread arg */
		thread_arg[ithread].thread_number = ithread;
		thread_arg[ithread].pbarrier = barrier;
		worker_t[ithread] = (HANDLE)_beginthreadex (NULL, 0, worker, 
			&thread_arg[ithread], 0, NULL);
		if (worker_t[ithread] == NULL) 
			ReportError (_T("Cannot create consumer thread"), 2, TRUE);
		Sleep(rand()/10);
	}
	
	/* Wait for the threads to complete */
	WaitForMultipleObjects (nthread, worker_t, TRUE, INFINITE);
	free (worker_t);
	_tprintf (_T("Worker threads have terminated\n"));
	free (thread_arg);
	_tprintf (_T("Result of closing THB. %d\n"), CloseThresholdBarrier (barrier));
	return 0;
}


DWORD WINAPI worker (PVOID arg)
{
	THARG * thread_arg;
	int ithread, tstatus;
	time_t nowa, nowd;

	thread_arg = (THARG *)arg;	
	ithread = thread_arg->thread_number;

	nowa = time(NULL);	
	_tprintf (_T("Start: Thread number %d %s"), ithread, ctime (&nowa));

	tstatus = WaitThresholdBarrier (thread_arg->pbarrier);
	if (tstatus != 0) ReportError (_T("Error waiting on at threshold barrier"), 9, TRUE);

	nowd = time(NULL);
	printf ("End:   Thread number %d %s", ithread, ctime (&nowd));
	return 0;		
}

