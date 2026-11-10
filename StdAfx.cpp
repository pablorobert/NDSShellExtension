// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#endif

// statreg.cpp / atlimpl.cpp existiam so no ATL 3 (VC6). O ATL atual e
// header-only, entao nao ha nada a incluir aqui.
#if _ATL_VER < 0x0700
#include <statreg.cpp>
#include <atlimpl.cpp>
#endif
