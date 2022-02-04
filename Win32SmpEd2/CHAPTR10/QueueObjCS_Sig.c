/* Chapter 10. QueueObjCS.c	CRITICAL_SECTION version			*/
/* Queue functions												*/
/* Use the "Signal model" (SetEvet/Auto-reset combination)			*/
/*	Exactly one waiting thread is released every time the queue		*/
/*	becomes non-empty or non-full. This works as long as the Q 		*/
/*	functions (q_get and q_put) only change the queue by one message*/
/*	at a time. If we released and/or obtained multiple messages		*/
/*	per get or put, then the broadcast model						*/
/*	(PulseEvent/Manual-reset) is necessary so that all waiting		*/
/*	threads will be released and can test the Queue state			*/
/*	As noted in the text, we use an INFINITE time out on the event	*/
/*	wait because the signal model does not have to worry about		*/
/*	lost signals													*/

#include "EvryThng.h"
#include "SynchObjCS.h"

/* Finite bounded queue management functions */

DWORD q_get (queue_t *q, PVOID msg, DWORD msize, DWORD MaxWait)
{
	DWORD TotalWaitTime = 0;
	BOOL TimedOut = FALSE;

	if (q_destroyed(q)) return 1;
//	WaitForSingleObject (q->q_guard, INFINITE);
	EnterCriticalSection (&q->q_guard);
	while (q_empty (q) && !TimedOut) {
//		ReleaseMutex (q->q_guard);
		LeaveCriticalSection (&q->q_guard);
		TimedOut = (WaitForSingleObject (q->q_ne, MaxWait) == WAIT_TIMEOUT);
//		WaitForSingleObject (q->q_guard, INFINITE);
		EnterCriticalSection (&q->q_guard);
	}
	/* remove the message from the queue */
	if (!TimedOut) q_remove (q, msg, msize);
	/* Signal that the queue is not full as we've removed a message */
	SetEvent (q->q_nf);
//	ReleaseMutex (q->q_guard);
	LeaveCriticalSection (&q->q_guard);

	return TimedOut ? WAIT_TIMEOUT : 0;
}

DWORD q_put (queue_t *q, PVOID msg, DWORD msize, DWORD MaxWait)
{
	DWORD TotalWaitTime = 0;
	BOOL TimedOut = FALSE;

	if (q_destroyed(q)) return 1;
//	WaitForSingleObject (q->q_guard, INFINITE);
	EnterCriticalSection (&q->q_guard);
	while (q_full (q) && !TimedOut) {
//		ReleaseMutex (q->q_guard);
		LeaveCriticalSection (&q->q_guard);
		TimedOut = (WaitForSingleObject (q->q_nf, MaxWait) == WAIT_TIMEOUT);
//		WaitForSingleObject (q->q_guard, INFINITE);
		EnterCriticalSection (&q->q_guard);
	}
	/* Put the message in the queue */
	if (!TimedOut) q_insert (q, msg, msize);	
	/* Signal that the queue is not empty as we've inserted a message */
	SetEvent (q->q_ne);
//	ReleaseMutex (q->q_guard);
	LeaveCriticalSection (&q->q_guard);
 
	return TimedOut ? WAIT_TIMEOUT : 0;
}

DWORD q_initialize (queue_t *q, DWORD msize, DWORD nmsgs)
{
	/* Initialize queue, including its mutex and events */
	/* Allocate storage for all messages. */
	
	q->q_first = q->q_last = 0;
	q->q_size = nmsgs;
	q->q_destroyed = 0;

	InitializeCriticalSection (&q->q_guard);
//	q->q_guard = CreateMutex (NULL, FALSE, NULL);
	q->q_ne = CreateEvent (NULL, FALSE, FALSE, NULL); /* Manual reset for signal model */
	q->q_nf = CreateEvent (NULL, FALSE, FALSE, NULL);

	if ((q->msg_array = calloc (nmsgs, msize)) == NULL) return 1;
	return 0; /* No error */
}

DWORD q_destroy (queue_t *q)
{
	if (q_destroyed(q)) return 1;
	/* Free all the resources created by q_initialize */
//	WaitForSingleObject (q->q_guard, INFINITE);
	EnterCriticalSection (&q->q_guard);
	q->q_destroyed = 1;
	free (q->msg_array);
	CloseHandle (q->q_ne);
	CloseHandle (q->q_nf);
//	ReleaseMutex (q->q_guard);
	LeaveCriticalSection (&q->q_guard);
//	CloseHandle (q->q_guard);
	DeleteCriticalSection (&q->q_guard);

	return 0;
}

DWORD q_destroyed (queue_t *q)
{
	return (q->q_destroyed);
}

DWORD q_empty (queue_t *q)
{
	return (q->q_first == q->q_last);
}

DWORD q_full (queue_t *q)
{
	return ((q->q_last - q->q_first) == 1 || 
		(q->q_first == q->q_size-1 && q->q_last == 0));
}


DWORD q_remove (queue_t *q, PVOID msg, DWORD msize)
{
	char *pm;
	
	pm = (char *)q->msg_array;
	/* Remove oldest ("first") message */
	memcpy (msg, pm + (q->q_first * msize), msize);
	q->q_first = ((q->q_first + 1) % q->q_size);
	return 0; /* no error */
}

DWORD q_insert (queue_t *q, PVOID msg, DWORD msize)
{
	char *pm;
	
	pm = (char *)q->msg_array;
	/* Add a new youngest ("last") message */
	if (q_full(q)) return 1; /* Error - Q is full */
	memcpy (pm + (q->q_last * msize), msg, msize);
	q->q_last = ((q->q_last + 1) % q->q_size);

	return 0;
}
