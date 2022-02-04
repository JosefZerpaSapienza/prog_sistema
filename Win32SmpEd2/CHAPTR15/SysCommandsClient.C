/* Chapter 15: SysCommandClient.c */
/* Server to call remote procedures to obtain system information */

#include "EvryThng.h"
#include <stdio.h>
#include <rpc.h>
#include "SysCommands.h"

int _tmain(DWORD argc, LPCTSTR argv[])
{
/*	Use the RPC try-except mechanism, which is similar to SEH (Chapter 6). */

	char Drive[] = "C:\\";
	int SecPerClus, BytesPerSec, NumFreeClus, Type;
	int VerInfoSize, MajVer, MinVer, BuildNum, PlatId;
	char ServicePack[128];

    RpcTryExcept {
		/* Get remote GetDiskFreeSpace information */
		get_disk_free_space ((unsigned char __RPC_FAR *)Drive, &SecPerClus, 
						  &BytesPerSec, &NumFreeClus);
		_tprintf ("Remote drive %s SecPerClus = %d. BytesPerSec = %d. NumFreeClus = %d\n", 
			Drive, SecPerClus, BytesPerSec, NumFreeClus);

		/* Get remote GetDriveType information */
		get_drive_type ((unsigned char __RPC_FAR *)Drive, &Type);
		_tprintf ("Remote drive type is: %d\n", Type);

		/* Get remote GetVersionEx information */
		get_os_version (&VerInfoSize, &MajVer, &MinVer, &BuildNum, &PlatId,
			ServicePack);
		_tprintf ("Remote Version %d.%d. BuildNum: %d. PlatId: %d\n",
			MajVer, MinVer, BuildNum, PlatId);

	}

    RpcExcept(1) 
		/* 1 is similar to EXCEPTION_EXECUTE_HANLDER */
		/* 0 is similar to EXCEPTION_CONTINUE_SEARCH */
	{
		_tprintf(_T("RPC runtime exception code = %d\n"), RpcExceptionCode());
    }
    RpcEndExcept
    
	return 0;
  }

/* System specific memory allocation and deallocation functions */

void __RPC_FAR* __RPC_API MIDL_user_allocate(size_t len)
{
    return((void __RPC_FAR*)malloc(len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR* ptr)
{
    free(ptr);
}


