

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Tue Mar 19 01:31:59 2013
 */
/* Compiler settings for .\NDSFileIcons.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __NDSFileIcons_h__
#define __NDSFileIcons_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __INDSIconShlExt_FWD_DEFINED__
#define __INDSIconShlExt_FWD_DEFINED__
typedef interface INDSIconShlExt INDSIconShlExt;
#endif 	/* __INDSIconShlExt_FWD_DEFINED__ */


#ifndef __NDSIconShlExt_FWD_DEFINED__
#define __NDSIconShlExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class NDSIconShlExt NDSIconShlExt;
#else
typedef struct NDSIconShlExt NDSIconShlExt;
#endif /* __cplusplus */

#endif 	/* __NDSIconShlExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __INDSIconShlExt_INTERFACE_DEFINED__
#define __INDSIconShlExt_INTERFACE_DEFINED__

/* interface INDSIconShlExt */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_INDSIconShlExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854AC32B-9A35-4494-A9B4-B62781F5F4D3")
    INDSIconShlExt : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct INDSIconShlExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INDSIconShlExt * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INDSIconShlExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INDSIconShlExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INDSIconShlExt * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INDSIconShlExt * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INDSIconShlExt * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INDSIconShlExt * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } INDSIconShlExtVtbl;

    interface INDSIconShlExt
    {
        CONST_VTBL struct INDSIconShlExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INDSIconShlExt_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define INDSIconShlExt_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define INDSIconShlExt_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define INDSIconShlExt_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define INDSIconShlExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define INDSIconShlExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define INDSIconShlExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __INDSIconShlExt_INTERFACE_DEFINED__ */



#ifndef __NDSFILEICONSLib_LIBRARY_DEFINED__
#define __NDSFILEICONSLib_LIBRARY_DEFINED__

/* library NDSFILEICONSLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_NDSFILEICONSLib;

EXTERN_C const CLSID CLSID_NDSIconShlExt;

#ifdef __cplusplus

class DECLSPEC_UUID("DF4F5AE4-E597-4C12-BC26-2776C27F71AE")
NDSIconShlExt;
#endif
#endif /* __NDSFILEICONSLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


