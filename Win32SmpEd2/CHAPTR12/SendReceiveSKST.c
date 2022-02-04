/*	SendReceiveSKST.c - Chapter 12. Multithreaded Streaming socket	*/
/*	DLL. Messages are delimited by end of line characters ('\0')		*/
/*	so the message length is not know ahead of time. Therefore, incoing	*/
/*	data is buffered and must be preserved from one function call to	*/
/*	the next. Therefore, we need to use Thread Local Storage (TLS)		*/
/*	so that each thread has its own privage "static storage"			*/
/*	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _NOEXCLUSIONS
#include "EvryThng.h"
#include "ClntSrvr.h"	/* Defines the request and response records. */

typedef struct STATIC_BUF_T {
/* "static_buf" contains "static_buf_len" characters of residual data */
/* There may or may not be end-of-string (null) characters */
	char static_buf[MAX_RQRS_LEN];
	LONG32 static_buf_len;
} STATIC_BUF;

static DWORD TlsIx = 0; /* index of TLS - EACH PROCESS HAS ITS OWN COPY */
/*	A single threaded library would use the following:
static char static_buf[MAX_RQRS_LEN];
static LONG32 static_buf_len;
*/

/* number of attached, detached threads and processes */
static long npa = 0, npd = 0, nta = 0, ntd = 0;

/* DLL main function. */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	STATIC_BUF * pBuf; 

	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			TlsIx = TlsAlloc();
			InterlockedIncrement (&npa); /* Overkill, as DllMain calls are serialized */
	/*	There is no thread attach call for the primary thread, so it is
		necessary to carry out the thread attach operations as well
		during process attach */

		case DLL_THREAD_ATTACH:
			/* Indicate that memory has not been allocated */
			InterlockedIncrement (&nta);
			TlsSetValue (TlsIx, NULL);
			return TRUE; /* This value is actually ignored */

		case DLL_PROCESS_DETACH:
		/* Detach the primary thread as well */
			InterlockedIncrement (&npd);
			/* Count this as detaching the primary thread as well */
			InterlockedIncrement (&ntd);
			pBuf = TlsGetValue (TlsIx);
			if (pBuf != NULL) {
				free (pBuf);
				pBuf = NULL;
			}

			printf ("Process detach. Counts: %d %d %d %d\n", npa, npd, nta, ntd);
			return TRUE;

		case DLL_THREAD_DETACH:
			InterlockedIncrement (&ntd);
			pBuf = TlsGetValue (TlsIx);
			if (pBuf != NULL) {
				free (pBuf);
				pBuf = NULL;
			}
			return TRUE;

		default: return TRUE;
	}
}


_declspec(dllexport)
BOOL ReceiveCSMessage (REQUEST *pRequest, SOCKET sd)
{
	/* TRUE return indicates an error or disconnect */
	BOOL Disconnect = FALSE;
	LONG32 nRemainRecv = 0, nXfer, k; /* Must be signed integers */
	LPSTR pBuffer, message;
	CHAR TempBuf[MAX_RQRS_LEN+1];
	STATIC_BUF *p;

	p = (STATIC_BUF *) TlsGetValue (TlsIx);
	if (p == NULL) { /* First time initialization. */
		/* Only threads that need this storage will allocate it */
		/* Other thread types can use the TLS for other purposes */
		p = malloc (sizeof (STATIC_BUF));
		TlsSetValue (TlsIx, p); 
		if (p == NULL) return TRUE; /* Error */
		p->static_buf_len = 0; /* Intialize state */
	}

	message = pRequest->Record; /* Requests and responses have the same structure */
	/*	Read up to the new line character, leaving residual data
	 *	in the static buffer */

	for (k = 0; k < p->static_buf_len && p->static_buf[k] != '\0'; k++) {
		message[k] = p->static_buf[k];
	}  /* k is the number of characters transferred */
	if (k < p->static_buf_len) { /* a null was found in static buf */
		message[k] = '\0'; 
		p->static_buf_len -= (k+1); /* Adjust the static buffer state */
		memcpy (p->static_buf, &(p->static_buf[k+1]), p->static_buf_len);
		return FALSE; /* No socket input required */
	} 
	
	/* the entire static buffer was transferred. No eol found */
	nRemainRecv = sizeof(TempBuf) - 1 - p->static_buf_len; 
	pBuffer = message + p->static_buf_len;
	p->static_buf_len = 0;

	while (nRemainRecv > 0 && !Disconnect) {
		nXfer = recv (sd, TempBuf, nRemainRecv, 0);
		if (nXfer <= 0) {
			Disconnect = TRUE;
			continue;
		}

		nRemainRecv -=nXfer;
		/* Transfer to target message up to null, if any */
		for (k = 0; k < nXfer && TempBuf[k] != '\0'; k++) {
			*pBuffer = TempBuf[k];
			pBuffer++;
		}
		if (k >= nXfer) { /* End of line not found, read more */
			nRemainRecv -= nXfer;
		} else { /* End of line has been found */
			*pBuffer = '\0';
			nRemainRecv = 0;
			memcpy (p->static_buf, &TempBuf[k+1], nXfer - k - 1);
			p->static_buf_len = nXfer -k - 1;
		}
	}
	return Disconnect;	
}


_declspec(dllexport)
BOOL SendCSMessage (RESPONSE *pResponse, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	BOOL Disconnect = FALSE;
	LONG32 nRemainSend, nXfer;
	LPSTR pBuffer;

	pBuffer = pResponse->Record;
	nRemainSend = strlen (pBuffer) + 1; 

	while (nRemainSend > 0 && !Disconnect)  {
		/* send does not guarantee that the entire message is sent */
		nXfer = send (sd, pBuffer, nRemainSend, 0);
		if (nXfer <= 0) {
			fprintf (stderr,
				"\nServer disconnect before complete request sent");
			Disconnect = TRUE;
		}
		nRemainSend -=nXfer; pBuffer += nXfer;
	}

	return Disconnect;
}
