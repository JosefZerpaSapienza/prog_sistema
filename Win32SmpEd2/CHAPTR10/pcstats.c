/* Chapter 10. pcstats.c						*/
/*												*/
/* DLL functions to accumulate and report statistics	*/
/* on the messages produced and comsumed			*/

#include <time.h>
#include "EvryThng.h"
#include "messages.h"
#include "pcstats.h"

/************************************************************************/

LIBSPEC void accumulate_statistics ()
{
	/* Compute statistics of the messages processed by this thread	*/
	/* The message block mutex must be locked			*/
	DWORD i, tcheck = 0;
	statistics_t * ps;
	msg_block_t *pmb;


	ps = TlsGetValue (ts_index);
	if (ps == NULL) ReportError ("Cannot get ts value", 21, TRUE);
	pmb = ps->pmblock;
	if (ps->nmsgs == 0) {
		ps->firstmsg = pmb->timestamp;
		ps->data_sum = 0.0;
	}
	ps->lastmsg = pmb->timestamp;
	ps->nmsgs++;
	
	for (i = 0; i < DATA_SIZE; i++) {
		tcheck |= pmb->data[i];
		ps->data_sum += (double)(pmb->data[i]);
	}
	
	if (tcheck == pmb->checksum)
		ps->ngood++;
	else
		ps->nbad++;		
	return;

}

LIBSPEC void report_statistics (void)
{ 
	/* Report on the work this thread performed */
	DWORD tcheck = 0;
	statistics_t * ps;
	double average, rate;
	time_t elapsed_time;

	ps = TlsGetValue (ts_index);
	printf ("\n*** Final report by worker thread number: %d\n",
		ps->th_number);
	printf ("    Messages processed = %6d. %6d good and %d bad\n",
		ps->nmsgs, ps->ngood, ps->nbad);
	printf ("    First message time: %s",   ctime (&ps->firstmsg));
	printf ("    Last  message time: %s", ctime (&ps->lastmsg ));
	elapsed_time = ps->lastmsg - ps->firstmsg;
	if (elapsed_time > 0) {
	rate = (double)ps->nmsgs / elapsed_time;
	printf ("Running time in seconds: %d. Message consumption rate = %10.2f\n",
		elapsed_time, rate);
	}
	
	if (ps->nmsgs > 0) {
		average = (ps->data_sum) /((ps->nmsgs) * DATA_SIZE);
		printf ("Average data item value: %15g\n", average);		
	}
}

