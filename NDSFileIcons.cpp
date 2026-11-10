// NDSFileIcons.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL,
//      run nmake -f TxtFileIconsps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "NDSFileIcons.h"
#include "NDSFileIcons_i.c"
#include "NDSIconShlExt.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_NDSIconShlExt, CNDSIconShlExt)
END_OBJECT_MAP()

static const TCHAR APPROVED_KEY[] =
    _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");
static const TCHAR CLSID_STR[] = _T("{DF4F5AE4-E597-4C12-BC26-2776C27F71AE}");

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_NDSFILEICONSLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow()
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// Registro por maquina ou por usuario
//
// Por maquina (precisa de admin): HKCR -> HKLM\Software\Classes, mais a
// lista "Approved" em HKLM.
// Por usuario (sem admin): HKCR e redirecionado para HKCU\Software\Classes
// com RegOverridePredefKey, assim o .rgs e o CRegKey escrevem la sem mudar
// nada. O Explorer (nao elevado) junta HKCU com HKLM e acha a extensao.

// true se da para escrever em HKLM (processo elevado / admin).
static bool CanWriteMachine()
{
    CRegKey key;
    return ERROR_SUCCESS == key.Open ( HKEY_LOCAL_MACHINE, APPROVED_KEY, KEY_SET_VALUE );
}

// Redireciona HKCR para HKCU\Software\Classes enquanto o objeto existir.
class CPerUserClassesRoot
{
public:
    CPerUserClassesRoot() : m_hKey(NULL), m_lRet(ERROR_SUCCESS)
    {
        m_lRet = RegCreateKeyEx ( HKEY_CURRENT_USER, _T("Software\\Classes"), 0, NULL, 0,
                                  KEY_ALL_ACCESS, NULL, &m_hKey, NULL );
        if ( ERROR_SUCCESS == m_lRet )
            m_lRet = RegOverridePredefKey ( HKEY_CLASSES_ROOT, m_hKey );
    }
    ~CPerUserClassesRoot()
    {
        RegOverridePredefKey ( HKEY_CLASSES_ROOT, NULL );
        if ( NULL != m_hKey )
            RegCloseKey ( m_hKey );
    }
    LONG Result() const { return m_lRet; }

private:
    HKEY m_hKey;
    LONG m_lRet;
};

// Guarda o DefaultIcon atual de .nds em OldDefIcon (uma vez so) para o
// unregister poder restaurar.
static void BackupDefaultIcon()
{
    CRegKey key;
    if ( ERROR_SUCCESS != key.Open ( HKEY_CLASSES_ROOT, _T(".nds\\DefaultIcon"),
                                     KEY_QUERY_VALUE | KEY_SET_VALUE ) )
        return;

    BYTE  buf[2 * MAX_PATH * sizeof(TCHAR)];
    DWORD dwSize = sizeof(buf);
    DWORD dwType;

    if ( ERROR_SUCCESS == RegQueryValueEx ( key.m_hKey, _T("OldDefIcon"), NULL, NULL, NULL, NULL ) )
        return;     // ja existe backup

    if ( ERROR_SUCCESS == RegQueryValueEx ( key.m_hKey, NULL, NULL, &dwType, buf, &dwSize ) )
        RegSetValueEx ( key.m_hKey, _T("OldDefIcon"), 0, dwType, buf, dwSize );
}

static void RestoreDefaultIcon()
{
    CRegKey key;
    if ( ERROR_SUCCESS != key.Open ( HKEY_CLASSES_ROOT, _T(".nds\\DefaultIcon"),
                                     KEY_QUERY_VALUE | KEY_SET_VALUE ) )
        return;

    BYTE  buf[2 * MAX_PATH * sizeof(TCHAR)];
    DWORD dwSize = sizeof(buf);
    DWORD dwType;

    if ( ERROR_SUCCESS == RegQueryValueEx ( key.m_hKey, _T("OldDefIcon"), NULL, &dwType, buf, &dwSize ) )
        {
        RegSetValueEx ( key.m_hKey, NULL, 0, dwType, buf, dwSize );
        key.DeleteValue ( _T("OldDefIcon") );
        }
}

static HRESULT RegisterImpl ( bool perUser )
{
    if ( perUser )
        {
        CPerUserClassesRoot redirect;
        if ( ERROR_SUCCESS != redirect.Result() )
            return HRESULT_FROM_WIN32 ( redirect.Result() );

        BackupDefaultIcon();
        return _Module.RegisterServer ( FALSE );
        }

    // "Approved": nome do valor = CLSID, dado = descricao. So tem efeito com
    // a policy EnforceShellExtensionSecurity, mas e o registro correto.
    CRegKey key;
    LONG lRet = key.Open ( HKEY_LOCAL_MACHINE, APPROVED_KEY, KEY_SET_VALUE );
    if ( ERROR_SUCCESS == lRet )
        lRet = key.SetStringValue ( CLSID_STR, _T("NDS file icon extension") );
    if ( ERROR_SUCCESS != lRet )
        return HRESULT_FROM_WIN32 ( lRet );
    key.Close();

    BackupDefaultIcon();
    return _Module.RegisterServer ( FALSE );
}

static HRESULT UnregisterImpl ( bool perUser )
{
    if ( perUser )
        {
        CPerUserClassesRoot redirect;
        if ( ERROR_SUCCESS != redirect.Result() )
            return HRESULT_FROM_WIN32 ( redirect.Result() );

        RestoreDefaultIcon();
        return _Module.UnregisterServer ( FALSE );
        }

    CRegKey key;
    if ( ERROR_SUCCESS == key.Open ( HKEY_LOCAL_MACHINE, APPROVED_KEY, KEY_SET_VALUE ) )
        {
        key.DeleteValue ( CLSID_STR );
        key.Close();
        }

    RestoreDefaultIcon();
    return _Module.UnregisterServer ( FALSE );
}

static void NotifyShell()
{
    SHChangeNotify ( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
//
//   regsvr32 NDSFileIcons.dll            -> maquina se for admin, senao usuario
//   regsvr32 /n /i:user NDSFileIcons.dll -> sempre so para o usuario

STDAPI DllRegisterServer()
{
    HRESULT hr = RegisterImpl ( !CanWriteMachine() );
    if ( SUCCEEDED(hr) )
        NotifyShell();
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry
//
// Remove o registro do usuario sempre e o da maquina quando houver admin.

STDAPI DllUnregisterServer()
{
    UnregisterImpl ( true );

    HRESULT hr = S_OK;
    if ( CanWriteMachine() )
        hr = UnregisterImpl ( false );

    NotifyShell();
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// DllInstall - regsvr32 /i:user (com ou sem /n)

STDAPI DllInstall ( BOOL bInstall, LPCWSTR pszCmdLine )
{
    bool perUser = NULL != pszCmdLine && 0 == _wcsicmp ( pszCmdLine, L"user" );
    if ( !perUser )
        perUser = !CanWriteMachine();

    HRESULT hr = bInstall ? RegisterImpl ( perUser ) : UnregisterImpl ( perUser );
    if ( SUCCEEDED(hr) )
        NotifyShell();
    return hr;
}
