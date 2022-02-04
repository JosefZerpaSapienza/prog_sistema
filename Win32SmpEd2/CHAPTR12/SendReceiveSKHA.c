/*	SendReceiveSKHA.c - Chapter 12. Multithreaded Streaming socket		*/
/*	This is a modification of SendReceiveSKST.c to illustrate a			*/
/*	different thread-safe library technique								*/
/*	State is preverved in a HANDLE-like state structure rather than		*/
/*	using TLS. This allows a thread to use several sockets on once		*/
/*	Messages are delimited by end of line characters ('\0')				*/
/*	so the message length is not know ahead of time. Therefore, incoming*/
/*	data is buffered and must be preserved from one function call to	*/
/*	the next. Therefore, we need to use a structure associated with 	*/
/*	each socket, and the socket has its own privage "static storage"	*/
/*	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _NOEXCLUSIONS
#include "EvryThng.h"
#include "ClntSrvr.h"	/* Defines the request and response records. */

typedef struct SOCKET_HANDLE_T {
/* Current socket state */
/* This struture contains "static_buf_len" characters of residual data */
/* There may or may not be end-of-string (null) characters */
	SOCKET sk;
	char static_buf[MAX_RQRS_LEN];
	LONG32 static_buf_len;
} SOCKET_HANDLE, * PSOCKET_HANDLE;

/*	Functions to create and close "streaming socket handles" */
_declspec (dllexport)
PVOID CreateCSSocketHandle (SOCKET s)
{
	PVOID p;
	PSOCKET_HANDLE ps;

	p = malloc (sizeof(SOCKET_HANDLE));
	if (p == NULL) return NULL;
	ps = (PSOCKET_HANDLE)p;
	ps->sk = s;
	ps->static_buf_len = 0; /* Initialize buffer state */
	return p;
}

_declspec (dllexport)
BOOL CloseCSSocketHandle (PVOID p)
{
	if (p == NULL) return FALSE;
	free (p);
	return TRUE;
}


_declspec(dllexport)
BOOL ReceiveCSMessage (REQUEST *pRequest, PVOID sh)
/*  Use a PVOID so that the calling program does not need to include the */
/*	SOCKET_HANDLE definition. */
{
	/* TRUE return indicates an error or disconnect */
	BOOL Disconnect = FALSE;
	LONG32 nRemainRecv = 0, nXfer, k; /* Must be signed integers */
	LPSTR pBuffer, message;
	CHAR TempBuf[MAX_RQRS_LEN+1];
	PSOCKET_HANDLE p;
	SOCKET sd;

	p = (PSOCKET_HANDLE)sh;
	if (p == NULL) return FALSE;
	sd = p->sk;

	/* This is all that's changed from SendReceiveSKST! */

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
BOOL SendCSMessage (RESPONSE *pResponse, PVOID sh)
{
	/* Send the the request to the server on socket sd */
	BOOL Disconnect = FALSE;
	LONG32 nRemainSend, nXfer;
	LPSTR pBuffer;
	SOCKET sd;
	PSOCKET_HANDLE p;

	p = (PSOCKET_HANDLE)sh;
	if (p == NULL) return FALSE;
	sd = p->sk;

	pBuffer = pResponse->Record;
	nRemainSend = strlen (pBuffer) + 1; 

	while (nRemainSend > 0 && !Disconnect)  {
		/* send does not guarantee that the entire message is sent */
		nXfer = send (sd, pBuffer, nRemainSend, 0);
		if (nXfer <= 0) {
			Disconnect = TRUE;
		}
		nRemainSend -=nXfer; pBuffer += nXfer;
	}

	return Disconnect;
}
