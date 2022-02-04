/* Chapter 15: SysCommandSesrver.c */
/* Server to implement remote procedures to obtain system information */


#include "EvryThng.h"
#include <rpc.h>      
#include "SysCommands.h"   /* shared header file created by MIDL compiler */


void  _tmain(DWORD argc, LPCTSTR argv[])
{
    RPC_BINDING_VECTOR * pBindVector;

    LPTSTR Endpoint = "1071"; 
		/* port number - Use \\pipe\pipename for named pipe */
    LPTSTR ProtocolSequence = "ncacn_ip_tcp"; 
		/* ncacn_np for named pipes */
    DWORD MaxCalls = RPC_C_PROTSEQ_MAX_REQS_DEFAULT;

    /* Establish the RPC network protocol */

    if (RpcServerUseProtseqEp(ProtocolSequence, MaxCalls, 
			Endpoint, NULL) != 0) 
		ReportError (_T("Error specifying protocol sequence"), 1, TRUE);  

	/* Register the interface so it can be located by clients */

    if (RpcServerRegisterIf(AboutRemoteSystem_v1_0_s_ifspec, NULL, NULL) != 0)
		ReportError (_T("Error registering interface"), 2, TRUE);

    /*	Register the program with the locator program. Two steps:
		1) Place exported function information in a binding vector. */

    if (RpcServerInqBindings(&pBindVector) != 0)
		ReportError (_T("Error getting binding handle"), 3, TRUE);

	/*	2) Export binding information to locator's name service
			database; clients can then import this binding info */

    if (RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT,
		"/.:/AboutRemoteSystem", AboutRemoteSystem_v1_0_s_ifspec,
				pBindVector, /* Set by RpcServerInqBindings */
				NULL) != 0)
					ReportError (_T("Error exporting binding"), 4, TRUE);
	
	/* The server is now registered and can now listen for incoming requests */
    printf("Server is now ready for client requests\n");
    if (RpcServerListen((long)1, MaxCalls, FALSE /* Wait for calls */) != 0)
		ReportError (_T("Error calling RpcServerListen"), 5, TRUE);
}


/*	System specific memory allocation and deallocation functions */
/*	Alternatively, you could allocate from heaps (see Chapter 6) */
void __RPC_FAR* __RPC_API MIDL_user_allocate(size_t len)
{
    return((void __RPC_FAR*)malloc(len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR* ptr)
{
    free(ptr);
}

/*	End of boilerplate code. Server-specific implementations */

/* Obtain the information returned by GetDiskFreeSpaceEx */
/* void get_disk_free_space (unsigned char __RPC_FAR *Drive, long *pSecPerClus, 
	will avoid a warning message. See SysCommands.h */

void get_disk_free_space (char *Drive, long *pSecPerClus, 
						  long *pBytesPerSec, long *pNumFreeClus)
{
	DWORD TotalNumClusters;
	GetDiskFreeSpace(Drive, pSecPerClus, pBytesPerSec, 
		pNumFreeClus, &TotalNumClusters);

	return;
} 

/* Obtain the information returned by GetDriveType */
void get_drive_type (char *Drive, int *pType)
{
	*pType = GetDriveType (Drive);
	return; 
}



/* Obtain the information returned by GetVersionEx */
void get_os_version (long *pOSVerInfoSize, long *pMajVer, 
		long *pMinVer, long *pBuildNum, long *pPlatId,
		unsigned char __RPC_FAR *ServicePack)
{
	OSVERSIONINFO VerInfo = {sizeof(OSVERSIONINFO)};
	
	if (!GetVersionEx(&VerInfo))
		ReportError (_T("GetVersionEx failed"), 0, TRUE);
	*pOSVerInfoSize = VerInfo.dwOSVersionInfoSize;
	*pMajVer = VerInfo.dwMajorVersion;
	*pMinVer = VerInfo.dwMinorVersion;
	*pBuildNum = VerInfo.dwBuildNumber;
	*pPlatId = VerInfo.dwPlatformId;
	strcpy (ServicePack, VerInfo.szCSDVersion);

	return;
}
