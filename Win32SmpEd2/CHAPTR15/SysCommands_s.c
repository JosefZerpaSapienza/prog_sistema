/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Mar 11 14:52:34 2000
 */
/* Compiler settings for Syscommands.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "Syscommands.h"

#define TYPE_FORMAT_STRING_SIZE   37                                
#define PROC_FORMAT_STRING_SIZE   55                                

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;

extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: AboutRemoteSystem, ver. 1.0,
   GUID={0x526d8629,0x98e7,0x4b5d,{0x9d,0x43,0xb8,0x65,0x6c,0xa0,0x18,0x6f}} */


extern RPC_DISPATCH_TABLE AboutRemoteSystem_v1_0_DispatchTable;

static const RPC_SERVER_INTERFACE AboutRemoteSystem___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x526d8629,0x98e7,0x4b5d,{0x9d,0x43,0xb8,0x65,0x6c,0xa0,0x18,0x6f}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &AboutRemoteSystem_v1_0_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE AboutRemoteSystem_v1_0_s_ifspec = (RPC_IF_HANDLE)& AboutRemoteSystem___RpcServerInterface;

extern const MIDL_STUB_DESC AboutRemoteSystem_StubDesc;

void __RPC_STUB
AboutRemoteSystem_get_disk_free_space(
    PRPC_MESSAGE _pRpcMessage )
{
    long __RPC_FAR *BytesPerSec;
    long __RPC_FAR *NumFreeClus;
    unsigned char __RPC_FAR *RootPathName;
    long __RPC_FAR *SecPerClus;
    long _M6;
    long _M7;
    long _M8;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &AboutRemoteSystem_StubDesc);
    
    ( unsigned char __RPC_FAR * )RootPathName = 0;
    ( long __RPC_FAR * )SecPerClus = 0;
    ( long __RPC_FAR * )BytesPerSec = 0;
    ( long __RPC_FAR * )NumFreeClus = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&RootPathName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        SecPerClus = &_M6;
        BytesPerSec = &_M7;
        NumFreeClus = &_M8;
        
        get_disk_free_space(
                       RootPathName,
                       SecPerClus,
                       BytesPerSec,
                       NumFreeClus);
        
        _StubMsg.BufferLength = 4U + 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *SecPerClus;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *BytesPerSec;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *NumFreeClus;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = 256;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RootPathName,
                        &__MIDL_TypeFormatString.Format[2] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
AboutRemoteSystem_get_drive_type(
    PRPC_MESSAGE _pRpcMessage )
{
    unsigned char __RPC_FAR *RootPathName;
    int __RPC_FAR *Type;
    int _M9;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &AboutRemoteSystem_StubDesc);
    
    ( unsigned char __RPC_FAR * )RootPathName = 0;
    ( int __RPC_FAR * )Type = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[18] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&RootPathName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[20],
                                           (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Type = &_M9;
        
        get_drive_type(RootPathName,Type);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( int __RPC_FAR * )_StubMsg.Buffer)++ = *Type;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = 256;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RootPathName,
                        &__MIDL_TypeFormatString.Format[16] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
AboutRemoteSystem_get_os_version(
    PRPC_MESSAGE _pRpcMessage )
{
    unsigned char __RPC_FAR *ServicePack;
    long _M10;
    long _M11;
    long _M12;
    long _M13;
    long _M14;
    MIDL_STUB_MESSAGE _StubMsg;
    long __RPC_FAR *pBuildNum;
    long __RPC_FAR *pMajorVersion;
    long __RPC_FAR *pMinorVersion;
    long __RPC_FAR *pOSVerInfoSize;
    long __RPC_FAR *pPlatId;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &AboutRemoteSystem_StubDesc);
    
    ( long __RPC_FAR * )pOSVerInfoSize = 0;
    ( long __RPC_FAR * )pMajorVersion = 0;
    ( long __RPC_FAR * )pMinorVersion = 0;
    ( long __RPC_FAR * )pBuildNum = 0;
    ( long __RPC_FAR * )pPlatId = 0;
    ( unsigned char __RPC_FAR * )ServicePack = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        pOSVerInfoSize = &_M10;
        pMajorVersion = &_M11;
        pMinorVersion = &_M12;
        pBuildNum = &_M13;
        pPlatId = &_M14;
        if(64 * 1 < 0)
            {
            RpcRaiseException(RPC_X_INVALID_BOUND);
            }
        ServicePack = NdrAllocate(&_StubMsg,64 * 1);
        
        get_os_version(
                  pOSVerInfoSize,
                  pMajorVersion,
                  pMinorVersion,
                  pBuildNum,
                  pPlatId,
                  ServicePack);
        
        _StubMsg.BufferLength = 4U + 4U + 4U + 4U + 4U + 12U;
        _StubMsg.MaxCount = 64;
        
        NdrConformantStringBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR *)ServicePack,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[30] );
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *pOSVerInfoSize;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *pMajorVersion;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *pMinorVersion;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *pBuildNum;
        
        *(( long __RPC_FAR * )_StubMsg.Buffer)++ = *pPlatId;
        
        _StubMsg.MaxCount = 64;
        
        NdrConformantStringMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                     (unsigned char __RPC_FAR *)ServicePack,
                                     (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[30] );
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = 64;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ServicePack,
                        &__MIDL_TypeFormatString.Format[26] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC AboutRemoteSystem_StubDesc = 
    {
    (void __RPC_FAR *)& AboutRemoteSystem___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
    0,
    0x50100a4, /* MIDL Version 5.1.164 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    1,  /* Flags */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION AboutRemoteSystem_table[] =
    {
    AboutRemoteSystem_get_disk_free_space,
    AboutRemoteSystem_get_drive_type,
    AboutRemoteSystem_get_os_version,
    0
    };
RPC_DISPATCH_TABLE AboutRemoteSystem_v1_0_DispatchTable = 
    {
    3,
    AboutRemoteSystem_table
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  2 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */
/*  4 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  6 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/*  8 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 10 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 12 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 16 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */
/* 18 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x10 ),	/* Type Offset=16 */
/* 22 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 26 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */
/* 28 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 30 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 32 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 34 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 36 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 40 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 42 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 44 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 46 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 48 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */
/* 52 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x0,	/* FC_RP */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x22,		/* FC_C_CSTRING */
			0x44,		/* FC_STRING_SIZED */
/*  8 */	0x40,		/* Corr desc:  constant, val=65536 */
			0x0,		/* 0 */
/* 10 */	NdrFcShort( 0x100 ),	/* 256 */
/* 12 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 14 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 16 */	
			0x11, 0x0,	/* FC_RP */
/* 18 */	NdrFcShort( 0x2 ),	/* Offset= 2 (20) */
/* 20 */	
			0x22,		/* FC_C_CSTRING */
			0x44,		/* FC_STRING_SIZED */
/* 22 */	0x40,		/* Corr desc:  constant, val=65536 */
			0x0,		/* 0 */
/* 24 */	NdrFcShort( 0x100 ),	/* 256 */
/* 26 */	
			0x11, 0x0,	/* FC_RP */
/* 28 */	NdrFcShort( 0x2 ),	/* Offset= 2 (30) */
/* 30 */	
			0x22,		/* FC_C_CSTRING */
			0x44,		/* FC_STRING_SIZED */
/* 32 */	0x40,		/* Corr desc:  constant, val=16384 */
			0x0,		/* 0 */
/* 34 */	NdrFcShort( 0x40 ),	/* 64 */

			0x0
        }
    };
