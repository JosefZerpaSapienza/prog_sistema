/* Chapter 9. statsMX.c										*/
/* Simple boss/worker system, where each worker reports		*/
/* its work output back to the boss for display.			*/
/* MUTEX VERSION											*/

#include "EvryThng.h"
#define DELAY_COUNT 20

/* Usage: statsMX nthread ntasks				*/
/* start up nthread worker threads, each assigned to perform	*/
/* "ntasks" work units. Each thread reports its progress		*/
/* in its own unshard slot in a work performed array			*/
/* ntasks is limited to 64K as it's an unsigned short int		*/
/* There is a reason for this, described in an exercise			*/

DWORD WINAPI worker (void *);

typedef struct _THARG {
	int thread_number;
	HANDLE *phMutex;
	unsigned short int tasks_to_complete;
	unsigned short int *tasks_complete;
} THARG;
	
int _tmain (DWORD argc, LPTSTR argv[])
{
	INT nthread, ithread;
	HANDLE *worker_t, hMutex;
	unsigned short int * task_count, tasks_per_thread;
	THARG * thread_arg;
	DWORD ThId;
	
	if (argc != 3) {
		_tprintf (_T("Usage: statsMX nthread ntasks\n"));
		return 1;
	}

	/* Create the mutex */
	hMutex = CreateMutex (NULL, FALSE, NULL);
		
	nthread = _ttoi(argv[1]);
	tasks_per_thread = _ttoi(argv[2]);
	worker_t = malloc (nthread * sizeof(HANDLE));
	task_count = calloc (nthread, sizeof (unsigned short int));
	thread_arg = calloc (nthread, sizeof (THARG));
	if (worker_t == NULL || task_count == NULL || thread_arg == NULL)
		ReportError (_T("Cannot allocate working memory"), 1, TRUE);
	
	for (ithread = 0; ithread < nthread; ithread++) {
		/* Fillin the thread arg */
		thread_arg[ithread].thread_number = ithread;
		thread_arg[ithread].tasks_to_complete = tasks_per_thread;
		thread_arg[ithread].tasks_complete = &task_count[ithread];
		thread_arg[ithread].phMutex = &hMutex;
		worker_t[ithread] = (HANDLE)_beginthreadex (NULL, 0, worker,
			&thread_arg[ithread], 0, &ThId);
		if (worker_t[ithread] == NULL) 
			ReportError (_T("Cannot create consumer thread"), 2, TRUE);
	}
	
	/* Wait for the threads to complete */
	for (ithread = 0; ithread < nthread; ithread += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects (min(nthread-ithread, MAXIMUM_WAIT_OBJECTS), 
					&worker_t[ithread], TRUE, INFINITE);

	free (worker_t);
	printf ("Worker threads have terminated\n");
	for (ithread = 0; ithread < nthread; ithread++) {
		_tprintf (_T("Tasks completed by thread %5d: %6d\n"), 
			ithread, task_count[ithread]);
	}
	return 0;
	free (task_count);
	free (thread_arg);
}


DWORD WINAPI worker (void *arg)
{
	THARG * thread_arg;
	int ithread;

	thread_arg = (THARG *)arg;	
	ithread = thread_arg->thread_number;
		
	while (*thread_arg->tasks_complete < thread_arg->tasks_to_complete) {
		delay_cpu (DELAY_COUNT);
		WaitForSingleObject (*(thread_arg->phMutex), INFINITE);
		(*thread_arg->tasks_complete)++;
		ReleaseMutex (*(thread_arg->phMutex)); 
	}
		
	return 0;		
}

