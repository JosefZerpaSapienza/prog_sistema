/* Chapter 10. ThreeStageCS.c	Uses CRITICAL_SECTION VERSION	*/
/* Three-stage Producer Consumer system							*/
/* Other files required in this project, either directly or		*/
/* in the form of libraries (DLLs are preferable)				*/
/*		QueueObj.c												*/
/*		Messages.c												*/
/*																*/
/*  Usage: ThreeStage npc goal  								*/
/* start up "npc" paired producer  and consumer threads.		*/
/* Each producer must produce a total of						*/
/* "goal" messages, where each message is tagged				*/
/* with the consumer that should receive it						*/
/* Messages are sent to a "transmitter thread" which performs	*/
/* additional processing before sending message groups to the	*/
/* "receiver thread." Finally, the receiver thread sends		*/
/* the messages to the consumer threads.						*/

/* Transmitter: Receive messages one at a time from producers,	*/
/* create a trasmission message of up to "TBLOCK_SIZE" messages	*/
/* to be sent to the Receiver. (this could be a network xfer	*/
/* Receiver: Take message blocks sent by the Transmitter		*/
/* and send the individual messages to the designated consumer	*/

#include "EvryThng.h"
#include "SynchObjCS.h" /* this is the ONLY change to the main program */
#include "messages.h"
#include <time.h>

#define DELAY_COUNT 1000
#define MAX_THREADS 1024

/* Queue lengths and blocking factors. These numbers are arbitrary and 	*/
/* can be adjusted for performance tuning. The current values are	*/
/* not well balanced.							*/

#define TBLOCK_SIZE 5  	/* Transmitter combines this many messages at at time */
#define TBLOCK_TIMEOUT 50 /* Transmiter timeout waiting for messages to block */
#define P2T_QLEN 10 	/* Producer to Transmitter queue length */
#define T2R_QLEN 4	/* Transmitter to Receiver queue length */
#define R2C_QLEN 4	/* Receiver to Consumer queue length - there is one
			 * such queue for each consumer */

DWORD WINAPI producer (PVOID);
DWORD WINAPI consumer (PVOID);
DWORD WINAPI transmitter (PVOID);
DWORD WINAPI receiver (PVOID);


typedef struct _THARG {
	volatile DWORD thread_number;
	volatile DWORD work_goal;    /* used by producers */
	volatile DWORD work_done;    /* Used by producers and consumers */
	char waste [8]; /* Assure there is no quadword overlap */
} THARG;


/* Grouped messages sent by the transmitter to receiver		*/
typedef struct t2r_msg_tag {
	volatile DWORD num_msgs; /* Number of messages contained	*/
	msg_block_t messages [TBLOCK_SIZE];
} t2r_msg_t;

queue_t p2tq, t2rq, *r2cq_array;

static volatile DWORD ShutDown = 0;
static DWORD EventTimeout = 50;

DWORD main (DWORD argc, char * argv[])
{
	DWORD tstatus, nthread, ithread, goal, thid;
	HANDLE *producer_th, *consumer_th, transmitter_th, receiver_th;
	THARG *producer_arg, *consumer_arg;
	
	if (argc < 3) {
		printf ("Usage: ThreeStage npc goal \n");
		return 1;
	}
	srand ((int)time(NULL));	/* Seed the RN generator */
	
	nthread = atoi(argv[1]);
	if (nthread > MAX_THREADS) {
		printf ("Maximum number of producers or consumers is %d.\n", MAX_THREADS);
		return 2;
	}
	goal = atoi(argv[2]);
	producer_th = malloc (nthread * sizeof(HANDLE));
	producer_arg = calloc (nthread, sizeof (THARG));
	consumer_th = malloc (nthread * sizeof(HANDLE));
	consumer_arg = calloc (nthread, sizeof (THARG));
	
	if (producer_th == NULL || producer_arg == NULL
	 || consumer_th == NULL || consumer_arg == NULL)
		ReportError ("Cannot allocate working memory for threads.", 1, FALSE);
	
	q_initialize (&p2tq, sizeof(msg_block_t), P2T_QLEN);
	q_initialize (&t2rq, sizeof(t2r_msg_t), T2R_QLEN);
	/* Allocate and initialize Receiver to Consumer queue for each consumer */
	r2cq_array = calloc (nthread, sizeof(queue_t));
	if (r2cq_array == NULL) ReportError ("Cannot allocate memory for r2c queues", 
					20, FALSE);
	
	for (ithread = 0; ithread < nthread; ithread++) {
		/* Initialize r2c queue for this consumer thread */
		q_initialize (&r2cq_array[ithread], sizeof(msg_block_t), R2C_QLEN);
		/* Fill in the thread arg */
		consumer_arg[ithread].thread_number = ithread;
		consumer_arg[ithread].work_goal = goal;
		consumer_arg[ithread].work_done = 0;

		consumer_th[ithread] = (HANDLE)_beginthreadex (NULL, 0,
				consumer, (PVOID)&consumer_arg[ithread], 0, &thid);
		if (consumer_th[ithread] == NULL) 
			ReportError ("Cannot create consumer thread", 2, TRUE);

		producer_arg[ithread].thread_number = ithread;
		producer_arg[ithread].work_goal = goal;
		producer_arg[ithread].work_done = 0;
		producer_th[ithread] = (HANDLE)_beginthreadex (NULL, 0,
			producer, (PVOID)&producer_arg[ithread], 0, &thid);
		if (producer_th[ithread] == NULL) 
			ReportError ("Cannot create producer thread", 3, TRUE);
	}

	transmitter_th = (HANDLE)_beginthreadex (NULL, 0, transmitter, NULL, 0, &thid);
	if (transmitter_th == NULL) 
		ReportError ("Cannot create tranmitter thread", 4, TRUE);
	receiver_th = (HANDLE)_beginthreadex (NULL, 0, receiver, NULL, 0, &thid);
	if (receiver_th == NULL) 
		ReportError ("Cannot create receiver thread", 5, TRUE);
	

	printf ("BOSS: All threads are running\n");	
	/* Wait for the producers to complete */
	/* The implementation allows too many threads for WaitForMultipleObjects */
	/* although you could call WFMO in a loop */
	for (ithread = 0; ithread < nthread; ithread++) {
		tstatus = WaitForSingleObject (producer_th[ithread], INFINITE);
		if (tstatus != 0) 
			ReportError ("Cannot wait for producer thread", 6, TRUE);
		printf ("BOSS: Producer %d produced %d work units\n",
			ithread, producer_arg[ithread].work_done);
	}
	/* Producers have completed their work. */
	printf ("BOSS: All producers have completed their work.\n");

	/* Wait for the consumers to complete */
	for (ithread = 0; ithread < nthread; ithread++) {
		tstatus = WaitForSingleObject (consumer_th[ithread], INFINITE);
		if (tstatus != 0) 
			ReportError ("Cannot wait for consumer thread", 7, TRUE);
		printf ("BOSS: consumer %d consumed %d work units\n",
			ithread, consumer_arg[ithread].work_done);
	}
	printf ("BOSS: All consumers have completed their work.\n");	

	ShutDown = 1; /* Set a shutdown flag */
	
	/* Terminate and wait for, the transmitter and receiver */
	/* This thread termination is tolerable (just barely) in this case */
	/* as the transmitter and */
	/* receiver cannot hold any resources other than a mutex */
	/* which will be abandoned. Can you do better? */

	TerminateThread (transmitter_th, 0);
	TerminateThread (receiver_th, 0);
	tstatus = WaitForSingleObject (transmitter_th, INFINITE);
	if (tstatus != WAIT_OBJECT_0) 
		ReportError ("Failed waiting for transmitter", 8, TRUE);
	tstatus = WaitForSingleObject (receiver_th, INFINITE);

	if (tstatus != WAIT_OBJECT_0) 
		ReportError ("Failed waiting for transmitter", 9, TRUE);

	q_destroy (&p2tq);
	q_destroy (&t2rq);
	for (ithread = 0; ithread < nthread; ithread++)
		q_destroy (&r2cq_array[ithread]);
	free (r2cq_array);
	free (producer_th); free (consumer_th);
	free (producer_arg); free(consumer_arg);
	printf ("System has finished. Shutting down\n");
	return 0;
}

DWORD WINAPI producer (PVOID arg)
{
	THARG * parg;
	DWORD ithread, tstatus;
	msg_block_t msg;
	
	parg = (THARG *)arg;	
	ithread = parg->thread_number;

	while (parg->work_done < parg->work_goal) { 
		/* Periodically produce work units until the goal is satisfied */
		/* messages receive a source and destination address which are */
		/* the same in this case but could, in general, be different. */
		delay_cpu (DELAY_COUNT * rand() / RAND_MAX);
		message_fill (&msg, ithread, ithread, parg->work_done);
		
		/* put the message in the queue */
		tstatus = q_put (&p2tq, &msg, sizeof(msg), INFINITE);
				
		parg->work_done++;		
	}

	return 0;		
}


DWORD WINAPI transmitter (PVOID arg)
{

	/* Obtain multiple producer messages, combining into a single	*/
	/* compound message for the receiver */

	DWORD tstatus, im;
	t2r_msg_t t2r_msg = {0};
	msg_block_t p2t_msg;

	while (!ShutDown) {
		t2r_msg.num_msgs = 0;
		/* pack the messages for transmission to the receiver */
		for (im = 0; im < TBLOCK_SIZE; im++) {
			tstatus = q_get (&p2tq, &p2t_msg, sizeof(p2t_msg), INFINITE);
			if (tstatus != 0) break;
			memcpy (&t2r_msg.messages[im], &p2t_msg, sizeof(p2t_msg));
			t2r_msg.num_msgs++;
		}

		tstatus = q_put (&t2rq, &t2r_msg, sizeof(t2r_msg), INFINITE);
		if (tstatus != 0) return tstatus;
	}
	return 0;
}


DWORD WINAPI receiver (PVOID arg)
{
	/* Obtain compund messages from the transmitter and unblock them	*/
	/* and transmit to the designated consumer.				*/

	DWORD tstatus, im, ic;
	t2r_msg_t t2r_msg;
	msg_block_t r2c_msg; 
	
	while (!ShutDown) {
		tstatus = q_get (&t2rq, &t2r_msg, sizeof(t2r_msg), INFINITE);
		if (tstatus != 0) return tstatus;
		/* Distribute the messages to the proper consumer */
		for (im = 0; im < t2r_msg.num_msgs; im++) {
			memcpy (&r2c_msg, &t2r_msg.messages[im], sizeof(r2c_msg));
			ic = r2c_msg.destination; /* Destination consumer */
			tstatus = q_put (&r2cq_array[ic], &r2c_msg, sizeof(r2c_msg), INFINITE);
			if (tstatus != 0) return tstatus;
		}

	}
	return 0;
}

DWORD WINAPI consumer (PVOID arg)
{
	THARG * carg;
	DWORD tstatus, ithread;
	msg_block_t msg;
	queue_t *pr2cq;

	carg = (THARG *) arg;
	ithread = carg->thread_number;
	
	carg = (THARG *)arg;	
	pr2cq = &r2cq_array[ithread];

	while (carg->work_done < carg->work_goal) { 
		/* Receive and display messages */
		
		tstatus = q_get (pr2cq, &msg, sizeof(msg), INFINITE);
		if (tstatus != 0) return tstatus;
				
//		printf ("\nMessage received by consumer #: %d", ithread);
//		message_display (&msg);

		carg->work_done++;		
	}

	return 0;
}
