/* ThbObjects.c. Program 10-2							*/
/* threshold barrier compound synch objects library		*/

#include "EvryThng.h"
#include "synchobj.h"

/**********************/
/*  THRESHOLD BARRIER OBJECTS */
/**********************/

DWORD CreateThresholdBarrier (THB_HANDLE *pthb, DWORD b_value)
{
	THB_HANDLE hthb;
	/* Initialize a barrier object */
	hthb = malloc (sizeof(THRESHOLD_BARRIER));
	if (hthb == NULL) return 1;

	hthb->b_guard = CreateMutex (NULL, FALSE, NULL);
	if (hthb->b_guard == NULL) return 2;
	
	hthb->b_broadcast = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (hthb->b_broadcast == NULL) return 3;
	
	hthb->b_threshold = b_value;
	hthb->b_count = 0;
	hthb->b_destroyed = 0;  /* the object is now valid. */

	*pthb = hthb;

	return 0;
}


DWORD WaitThresholdBarrier (THB_HANDLE thb)
{
	/* Wait for the specified number of thread to reach */
	/* the barrier, then broadcast on the CV */

	/* Assure that the object is still valid */
	if (thb->b_destroyed == 1) return 1;
	
	WaitForSingleObject (thb->b_guard, INFINITE);
	thb->b_count++;  /* A new thread has arrived */
	while (thb->b_count < thb->b_threshold) {
		ReleaseMutex (thb->b_guard);
		WaitForSingleObject (thb->b_broadcast, CV_TIMEOUT);
		WaitForSingleObject (thb->b_guard, INFINITE);
	}
	SetEvent (thb->b_broadcast); /* Broadcast to all waiting threads */
	ReleaseMutex (thb->b_guard);	
	return 0;
}


DWORD CloseThresholdBarrier (THB_HANDLE thb)
{
	/* Destroy the component mutexes and event once it is safe to do so */
	/* This is not entirely satisfactory, but is consistent with what would */
	/* happen if a mutex handle were destroyed while another thread waited on it */
	if (thb->b_destroyed == 1) return 1;
	WaitForSingleObject (thb->b_guard, INFINITE);
	thb->b_destroyed = 1; /* No thread will try to use it  during this time inteval. */

	/* Now release the mutex and close the handle */
	ReleaseMutex (thb->b_guard);
	CloseHandle (thb->b_broadcast);
	CloseHandle (thb->b_guard);
	free (thb);
	return 0;

}
