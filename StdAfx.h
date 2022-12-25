// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__4FE97F75_F8A7_4302_BCBC_7051D202E8C3__INCLUDED_)
#define AFX_STDAFX_H__4FE97F75_F8A7_4302_BCBC_7051D202E8C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WINVER          0x0600
#define _WIN32_WINNT    0x0600
#define _ATL_APARTMENT_THREADED

// ATL
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlconv.h>

// Win32
#include <comdef.h>
#include <shlobj.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4FE97F75_F8A7_4302_BCBC_7051D202E8C3__INCLUDED)
