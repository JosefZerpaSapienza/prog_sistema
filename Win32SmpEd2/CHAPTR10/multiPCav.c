/* Chapter 10. multiPCav.c				*/
/* YOUR TASK: This is a Pthreads program. Convert to Win32 	*/
/* Maintain a producer and several consumers			*/
/* The producer  creates checksummed data buffers, 		*/
/* or "message blocks" which the consumers process, generating	*/
/* statistics. The conusmer reads the NEXT complete	 	*/
/* set of data and validates it before display			*/
/* The producer does not create new data until the previous	*/
/* buffer has been consumed; it will be consumed by a single	*/
/* consumer thread.						*/
/* The main parent thread creates new consumers on demand	*/
/* requiring thread-specific storage and one-time initializaton	*/

#include <pthread.h>
#include "pcstats.h"
#include "errors.h"
#include <stdio.h>

#define MAX_CONSUMER 128

/* The invariant and condition variable predicates are:		*/
/*	Invariant - One of the following conditions holds 	*/
/*	  1)	f_consumed 					*/
/*			 nothing is assured about the data	*/
/*	  2)	(f_stop || f_ready) && data is valid			*/
/*	   && checksum and timestamp are valid			*/ 
/*	Condition variable predicates				*/
/*	  1)	mconsumed if and only if f_consumed || f_stop	*/
/*	  2)	mready if and only if f_ready			*/


/* Single message block, ready to fill with a new message 	*/
struct msg_block_tag mblock = { PTHREAD_MUTEX_INITIALIZER, 
	PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 
	1, 0, 0, 0 }; 

void * produce (void *), * consume (void *);
void message_fill (msg_block_t *);

void accumulate_statistics ();
void report_statistics();
void once_init_function (void);

int main (int argc, char * argv[])
{
	int tstatus, nc = 0, ic;
	pthread_t produce_t, consume_t;
	char command, extra;
	pthread_t consumer_thread [MAX_CONSUMER];
		
	/* Create the producer thread */
	tstatus = pthread_create (&produce_t, NULL, produce, NULL);
	if (tstatus != 0) 
		err_abort (tstatus, "Cannot create producer thread");
		
	/* Prompt the user to create a consumer or shutdown */
	while (!mblock.f_stop) { /* This is the only thread accessing stdin, stdout */
		printf ("\n**Enter 'c' for create consumer; 's' to stop: ");
		scanf ("%c%c", &command, &extra);
		if (command == 's') {
			pthread_mutex_lock (&mblock.mguard);
			mblock.f_stop = 1;
			pthread_cond_signal (&mblock.mready);
			pthread_mutex_unlock (&mblock.mguard);
		} else if (command == 'c' && nc < MAX_CONSUMER-1) { 
			tstatus = pthread_create 
				(&consumer_thread[nc++], NULL, consume, &mblock);
			if (tstatus != 0) 
				err_abort (tstatus, "Cannot create consumer thread");
		} else {
			printf ("Illegal command or too many consumers.\n");
		}
	}
		
	printf ("\nConsumers will report their results and terminate\n\n");
	
	/* Wait for the producer to terminate */
	tstatus = pthread_join (produce_t, NULL);
	if (tstatus != 0) 
		err_abort (tstatus, "Cannot join producer thread\n");

	/* Wait for the consumer threads to complete */
	for (ic = 0; ic < nc; ic++) {
		tstatus = pthread_join (consumer_thread[ic], NULL);
		if (tstatus != 0) 
			err_abort (tstatus, "Cannot join consumer thread");
	}
	
	printf ("Producer and consumer threads have terminated\n");
	return 0;
}

void * produce (void *arg)
/* Producer thread - Create new messages whenever a conusumer is ready */
{
	int nproduced = 0;
	
	while (!mblock.f_stop) {
		
		/* Get the buffer, wait for it to be safe to fill, fill it,*/
		/* and inform the consumer					*/
		pthread_mutex_lock (&mblock.mguard);
		while (!mblock.f_consumed && !mblock.f_stop)
			pthread_cond_wait (&mblock.mconsumed, &mblock.mguard);
/*		printf ("\nProducers consumed wait return\n"); */
		if (!mblock.f_stop) {
			message_fill (&mblock);
			mblock.sequence++;
			mblock.f_consumed = 0;
			mblock.f_ready = 1;
			nproduced++;
/*			printf ("Message produced\n"); */
		}
		pthread_cond_signal (&mblock.mready);
		pthread_mutex_unlock (&mblock.mguard);
	}
	printf ("Producer shutting down after producing %d messages\n", nproduced);
	return NULL;
}

void message_fill (msg_block_t *mblock)
{
	/* Fill the message buffer, and include checksum and timestamp	*/
	/* This function is called from the producer thread while it 	*/
	/* owns the message block mutex					*/
	
	int i;
	
	mblock->checksum = 0;	
	for (i = 0; i < DATA_SIZE; i++) {
		mblock->data[i] = rand();
		mblock->checksum ^= mblock->data[i];
	}
	mblock->timestamp = time(NULL);
	return;
}

void *consume (void *arg)
/* Consumer thread function. */
{
	msg_block_t *pmb;
	statistics_t * ps;
	int my_number, tstatus;
	struct timespec timeout, delta;
	
	delta.tv_sec = 2;
	delta.tv_nsec = 0;
		
	/* Create thread-specific storage key */
	tstatus = pthread_once (&once_control, once_init_function);
	if (tstatus != 0) err_abort (tstatus, "One time init failed");

	pmb = (msg_block_t *)arg;

	/* Allocate storage for thread-specific statistics */
	ps = calloc (sizeof(statistics_t), 1);
	if (ps == NULL) errno_abort ("Cannot allocate memory");
	tstatus = pthread_setspecific (ts_key, ps);
	if (tstatus != 0) err_abort (tstatus, "Error setting ts storage");
	ps->pmblock = pmb;
	/* Give this thread a unique number */
	/* Note that the mutex is "overloaded" to protect data	*/
	/* outside the message block structure			*/
	tstatus = pthread_mutex_lock (&pmb->mguard);
	if (tstatus != 0) err_abort (tstatus, "Lock error");
	ps->th_number = thread_number++;
	tstatus = pthread_mutex_unlock (&pmb->mguard);
	if (tstatus != 0) err_abort (tstatus, "Unlock error");
	
	/* Consume the NEXT message when prompted by the user */
	while (!pmb->f_stop) { /* This is the only thread accessing stdin, stdout */
		tstatus = pthread_mutex_lock (&pmb->mguard);
		if (tstatus != 0) err_abort (tstatus, "Lock error");
		/* Get the next message. Use a timed wait so as to be able	*/
		/* to sample the stop flag peridically.				*/
		do { 
			pthread_get_expiration_np (&delta, &timeout);
			tstatus = pthread_cond_timedwait 
				(&pmb->mready, &pmb->mguard, &timeout);
			if (tstatus != 0 && tstatus != ETIMEDOUT) 
				err_abort (tstatus, "CV wait error");
		} while (!pmb->f_ready && !pmb->f_stop);
		if (!pmb->f_stop) {
/*			printf ("Message received\n"); */
			accumulate_statistics ();
			pmb->f_consumed = 1;
			pmb->f_ready = 0;
		}
		tstatus = pthread_cond_signal (&pmb->mconsumed);
		if (tstatus != 0) err_abort (tstatus, "Signal error");
		tstatus = pthread_mutex_unlock (&pmb->mguard);
		if (tstatus != 0) err_abort (tstatus, "Unlock error");
	}
	
	/* Shutdown. Report the statistics */
	tstatus = pthread_mutex_lock (&pmb->mguard);
	if (tstatus != 0) err_abort (tstatus, "Lock error");
	report_statistics ();	
	tstatus = pthread_mutex_unlock (&pmb->mguard);
	if (tstatus != 0) err_abort (tstatus, "Unlock error");

	/* Terminate the consumer thread. The destructor will 	*/
	/* free the memory allocated for the statistics		*/
	return NULL;		
}

