/* Chapter 4.  Excption.c
	Generate exceptions intentionally and respond to them. */

#include "EvryThng.h"
#include <float.h>

DWORD Filter (LPEXCEPTION_POINTERS, LPDWORD);
double x = 1.0, y = 0.0;
int _tmain (int argc, LPTSTR argv [])
{
	DWORD ECatgry, i = 0, ix, iy = 0;
	LPDWORD pNull = NULL;
	BOOL Done = FALSE;
	DWORD FPOld, FPNew;
	//__try { /* Try-Finally block. */
						/* Save old control mask. */
	FPOld = _controlfp (0, 0);
						/* Enable floating-point exceptions. */
	FPNew = FPOld & ~(EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT
			| EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);
						/* Set new control mask. */
	_controlfp (FPNew, MCW_EM);
	while (!Done) __try {
		_tprintf (_T ("Enter exception type:\n"));
		_tprintf (_T ("1: Mem, 2: Int, 3: Flt 4: User 5: _leave 6: return\n"));
		_tscanf (_T ("%d"), &i);
		__try { /* Try-Except block. */
			switch (i) {
			case 1: /* Memory reference. */
				ix = *pNull;
				*pNull = 5;
				break;
			case 2: /* Integer arithmetic. */
				ix = ix / iy;
				break;
			case 3: /* floating-point exception. */
				x = x / y;
				_tprintf (_T ("x = %20e\n"), x);
				break;
			case 4: /* User generated exception. */
				ReportException (_T ("Raising user exception.\n"), 1);
				break;
			case 5: /* Use the _leave statement to terminate. */
				Done = TRUE;
				__leave;
			case 6: /* Use the _leave statement to terminate. */
				return 1;
			default: Done = TRUE;
			}
		} /* End of inner __try. */

		__except (Filter (GetExceptionInformation (), &ECatgry)){
			switch (ECatgry) {
			case 0:	_tprintf (_T ("Unknown exception.\n"));
				break;
			case 1:	_tprintf (_T ("Memory ref exception.\n"));
				break;
			case 2:	_tprintf (_T ("Integer arithmetic exception.\n"));
				break;
			case 3:	
	 			_tprintf (_T ("floating-point exception.\n"));
				break;
			case 10: _tprintf (_T ("User generated exception.\n"));
				break; 
			default: _tprintf (_T ("Unknown exception.\n"));
				break;
			}
			_tprintf (_T ("End of handler.\n"));
		} /* End of inner __try __except block. */


	
	//} /* End of exception generation loop. */

	//return; /* Cause an abnormal termination. */

	} /* End of outer __try __finally */
	__finally {
		BOOL AbTerm; /* Restore the old mask value. */
		_controlfp (FPOld, MCW_EM);
		AbTerm = AbnormalTermination();
		_tprintf (_T ("Abnormal Termination?: %d\n"), !AbTerm);
 	}

}

static DWORD Filter (LPEXCEPTION_POINTERS pExP, LPDWORD ECatgry)

/*	Categorize the exception and decide whether to continue execution or
	execute the handler or to continue the search for a handler that
	can process this exception type. The exception category is only used
	by the exception handler. */
{
	DWORD ExCode, ReadWrite, VirtAddr;
	ExCode = pExP->ExceptionRecord->ExceptionCode;
	_tprintf (_T ("Filter. ExCode: %x\n"), ExCode);
	if ((0x20000000 & ExCode) != 0) {
				/* User Exception. */
		*ECatgry = 10;
		return EXCEPTION_EXECUTE_HANDLER;
	}

	switch (ExCode) {
		case EXCEPTION_ACCESS_VIOLATION:
				/* Determine whether it was a read or write
					and give the virtual address. */
			ReadWrite =
				pExP->ExceptionRecord->ExceptionInformation [0];
			VirtAddr =
				pExP->ExceptionRecord->ExceptionInformation [1];
			_tprintf
				("Access Violation. Read/Write: %d. Address: %x\n",
				ReadWrite, VirtAddr);
			*ECatgry = 1;
			return EXCEPTION_EXECUTE_HANDLER;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			*ECatgry = 1;
			return EXCEPTION_EXECUTE_HANDLER;
					/* Integer arithmetic exception. Halt execution. */
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
			*ECatgry = 2;
			return EXCEPTION_EXECUTE_HANDLER;
					/* Float exception. Attempt to continue execution. */
					/* Return the maximum floating value. */
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_OVERFLOW:
			_tprintf (_T ("Flt Exception - Large result.\n"));
			*ECatgry = 3;
			_clearfp();
			return (DWORD) EXCEPTION_EXECUTE_HANDLER;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_STACK_CHECK:
			_tprintf (_T ("Flt Exception - Unknown result.\n"));
			*ECatgry = 3;
			return (DWORD) EXCEPTION_CONTINUE_EXECUTION;
					/* Return the minimum floating value. */
		case EXCEPTION_FLT_UNDERFLOW:
			_tprintf (_T ("Flt Exception - Small result.\n"));
			*ECatgry = 3;
			return (DWORD) EXCEPTION_CONTINUE_EXECUTION;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			*ECatgry = 4;
			return EXCEPTION_CONTINUE_SEARCH;
		case STATUS_NONCONTINUABLE_EXCEPTION:
			*ECatgry = 5;
			return EXCEPTION_EXECUTE_HANDLER;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_PRIV_INSTRUCTION:
			*ECatgry = 6;
			return EXCEPTION_EXECUTE_HANDLER;
		case STATUS_NO_MEMORY:
			*ECatgry = 7;
			return EXCEPTION_EXECUTE_HANDLER;
		default:
			*ECatgry = 0;
			return EXCEPTION_CONTINUE_SEARCH;
	}
}
