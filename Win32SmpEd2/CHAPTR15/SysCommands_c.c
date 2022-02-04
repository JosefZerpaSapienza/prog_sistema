/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Mar 11 14:52:34 2000
 */
/* Compiler settings for Syscommands.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

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



static const RPC_CLIENT_INTERFACE AboutRemoteSystem___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x526d8629,0x98e7,0x4b5d,{0x9d,0x43,0xb8,0x65,0x6c,0xa0,0x18,0x6f}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE AboutRemoteSystem_v1_0_c_ifspec = (RPC_IF_HANDLE)& AboutRemoteSystem___RpcClientInterface;

extern const MIDL_STUB_DESC AboutRemoteSystem_StubDesc;

static RPC_BINDING_HANDLE AboutRemoteSystem__MIDL_AutoBindHandle;


void get_disk_free_space( 
    /* [size_is][string][in] */ unsigned char __RPC_FAR *RootPathName,
    /* [out] */ long __RPC_FAR *SecPerClus,
    /* [out] */ long __RPC_FAR *BytesPerSec,
    /* [out] */ long __RPC_FAR *NumFreeClus)
{

    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    if(!RootPathName)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!SecPerClus)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!BytesPerSec)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!NumFreeClus)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    RpcTryFinally
        {
        NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&AboutRemoteSystem_StubDesc,
                          0);
        
        
        
        _StubMsg.BufferLength = 12U;
        _StubMsg.MaxCount = 256;
        
        NdrConformantStringBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR *)RootPathName,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6] );
        
        NdrNsGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, AboutRemoteSystem__MIDL_AutoBindHandle );
        
        _StubMsg.MaxCount = 256;
        
        NdrConformantStringMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                     (unsigned char __RPC_FAR *)RootPathName,
                                     (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6] );
        
        NdrNsSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer, (RPC_BINDING_HANDLE __RPC_FAR *) &AboutRemoteSystem__MIDL_AutoBindHandle );
        
        if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        *SecPerClus = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        *BytesPerSec = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        *NumFreeClus = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        }
    RpcFinally
        {
        NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
        
        }
    RpcEndFinally
    
}


void get_drive_type( 
    /* [size_is][string][in] */ unsigned char __RPC_FAR *RootPathName,
    /* [out] */ int __RPC_FAR *Type)
{

    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    if(!RootPathName)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!Type)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    RpcTryFinally
        {
        NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&AboutRemoteSystem_StubDesc,
                          1);
        
        
        
        _StubMsg.BufferLength = 12U;
        _StubMsg.MaxCount = 256;
        
        NdrConformantStringBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR *)RootPathName,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[20] );
        
        NdrNsGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, AboutRemoteSystem__MIDL_AutoBindHandle );
        
        _StubMsg.MaxCount = 256;
        
        NdrConformantStringMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                     (unsigned char __RPC_FAR *)RootPathName,
                                     (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[20] );
        
        NdrNsSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer, (RPC_BINDING_HANDLE __RPC_FAR *) &AboutRemoteSystem__MIDL_AutoBindHandle );
        
        if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[18] );
        
        *Type = *(( int __RPC_FAR * )_StubMsg.Buffer)++;
        
        }
    RpcFinally
        {
        NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
        
        }
    RpcEndFinally
    
}


void get_os_version( 
    /* [out] */ long __RPC_FAR *pOSVerInfoSize,
    /* [out] */ long __RPC_FAR *pMajorVersion,
    /* [out] */ long __RPC_FAR *pMinorVersion,
    /* [out] */ long __RPC_FAR *pBuildNum,
    /* [out] */ long __RPC_FAR *pPlatId,
    /* [size_is][string][out] */ unsigned char __RPC_FAR *ServicePack)
{

    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    if(!pOSVerInfoSize)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!pMajorVersion)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!pMinorVersion)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!pBuildNum)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!pPlatId)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!ServicePack)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    RpcTryFinally
        {
        NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&AboutRemoteSystem_StubDesc,
                          2);
        
        
        
        _StubMsg.BufferLength = 0U;
        NdrNsGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, AboutRemoteSystem__MIDL_AutoBindHandle );
        
        NdrNsSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer, (RPC_BINDING_HANDLE __RPC_FAR *) &AboutRemoteSystem__MIDL_AutoBindHandle );
        
        if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[28] );
        
        *pOSVerInfoSize = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        *pMajorVersion = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        *pMinorVersion = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        *pBuildNum = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        *pPlatId = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ServicePack,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[30],
                                       (unsigned char)0 );
        
        }
    RpcFinally
        {
        NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
        
        }
    RpcEndFinally
    
}


static const MIDL_STUB_DESC AboutRemoteSystem_StubDesc = 
    {
    (void __RPC_FAR *)& AboutRemoteSystem___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &AboutRemoteSystem__MIDL_AutoBindHandle,
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
