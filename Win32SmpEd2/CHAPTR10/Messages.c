/* Chapter 10. messages.c										*/
/* Queue functions												*/

#include "EvryThng.h"
#include "messages.h"
#include <time.h>

DWORD compute_checksum (PVOID msg, DWORD length)
{
	/* Computer an xor checksum on the entire message of "length"
	 * integers */
	DWORD i, cs = 0, *pint;

	pint = (DWORD *) msg;
	for (i = 0; i < length; i++) {
		cs = (cs ^ *pint);
		pint++;
	}
	return cs;		
}

VOID message_fill (msg_block_t *mblock, DWORD src, DWORD dest, DWORD seqno)
{
	/* Fill the message buffer, and include checksum and timestamp	*/
	/* This function is called from the producer thread while it 	*/
	/* owns the message block mutex					*/

	DWORD i;
	
	mblock->checksum = 0;	
	for (i = 0; i < DATA_SIZE; i++) {
		mblock->data[i] = rand();
	}
	mblock->source = src;
	mblock->destination = dest;
	mblock->sequence = seqno;
	mblock->timestamp = time(NULL);
	mblock->checksum = compute_checksum (mblock, sizeof(msg_block_t)/sizeof(DWORD));
/*	printf ("Generated message: %d %d %d %d %x %x\n", 
		src, dest, seqno, mblock->timestamp, 
		mblock->data[0], mblock->data[DATA_SIZE-1]);  */
	return;
}


void message_display (msg_block_t *mblock)
{
	/* Display message buffer and timestamp, validate checksum	*/
	/* This function is called from the consumer thread while it 	*/
	/* owns the message block mutex					*/
	DWORD tcheck = 0;
	
	tcheck = compute_checksum (mblock, sizeof(msg_block_t)/sizeof(DWORD));
	printf ("\nMessage number %d generated at: %s", 
		mblock->sequence, ctime (&(mblock->timestamp)));
	printf ("Source and destination: %d %d\n", 
		mblock->source, mblock->destination);
	printf ("First and last entries: %x %x\n",
		mblock->data[0], mblock->data[DATA_SIZE-1]);
	if (tcheck == 0 /*mblock->checksum was 0 when CS first computed */)
		printf ("GOOD ->Checksum was validated.\n");
	else
		printf ("BAD  ->Checksum failed. message was corrupted\n");
		
	return;

}
