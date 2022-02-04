/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Mar 11 14:52:34 2000
 */
/* Compiler settings for Syscommands.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __Syscommands_h__
#define __Syscommands_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __AboutRemoteSystem_INTERFACE_DEFINED__
#define __AboutRemoteSystem_INTERFACE_DEFINED__

/* interface AboutRemoteSystem */
/* [auto_handle][unique][version][uuid] */ 

void get_disk_free_space( 
    /* [size_is][string][in] */ unsigned char __RPC_FAR *RootPathName,
    /* [out] */ long __RPC_FAR *SecPerClus,
    /* [out] */ long __RPC_FAR *BytesPerSec,
    /* [out] */ long __RPC_FAR *NumFreeClus);

void get_drive_type( 
    /* [size_is][string][in] */ unsigned char __RPC_FAR *RootPathName,
    /* [out] */ int __RPC_FAR *Type);

void get_os_version( 
    /* [out] */ long __RPC_FAR *pOSVerInfoSize,
    /* [out] */ long __RPC_FAR *pMajorVersion,
    /* [out] */ long __RPC_FAR *pMinorVersion,
    /* [out] */ long __RPC_FAR *pBuildNum,
    /* [out] */ long __RPC_FAR *pPlatId,
    /* [size_is][string][out] */ unsigned char __RPC_FAR *ServicePack);



extern RPC_IF_HANDLE AboutRemoteSystem_v1_0_c_ifspec;
extern RPC_IF_HANDLE AboutRemoteSystem_v1_0_s_ifspec;
#endif /* __AboutRemoteSystem_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
