// NDSFileIcons.cpp : Implementation of DLL Exports.

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
// Por maquina (precisa de admin): HKCR e redirecionado para
// HKLM\Software\Classes, mais a lista "Approved" em HKLM.
// Por usuario (sem admin): HKCR e redirecionado para HKCU\Software\Classes.
// O redirecionamento (RegOverridePredefKey) faz o .rgs e o CRegKey
// escreverem exatamente na raiz escolhida, sem cair na outra pela visao
// mesclada do HKCR. O Explorer (nao elevado) junta HKCU com HKLM.
//
// O IconHandler e o DefaultIcon=%1 sao gravados em ".nds" e tambem no ProgID
// que for dono do .nds no momento (default de HKCR\.nds e UserChoice). Sem
// isso, um emulador que assume o .nds faz o Explorer ignorar o handler.
// Nao criamos ProgID proprio para nao tomar a associacao de abertura do
// emulador. Cada chave tocada fica listada em CLSID\{...}\Bindings para o
// unregister desfazer exatamente o que foi feito.

static const TCHAR BINDINGS_KEY[] =
    _T("CLSID\\{DF4F5AE4-E597-4C12-BC26-2776C27F71AE}\\Bindings");
static const TCHAR USERCHOICE_KEY[] =
    _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.nds\\UserChoice");
static const TCHAR OLD_DEFICON[]     = _T("OldDefIcon");
static const TCHAR OLD_ICONHANDLER[] = _T("OldIconHandler");

// true se da para escrever em HKLM (processo elevado / admin).
static bool CanWriteMachine()
{
    CRegKey key;
    return ERROR_SUCCESS == key.Open ( HKEY_LOCAL_MACHINE, APPROVED_KEY, KEY_SET_VALUE );
}

// Redireciona HKCR para <root>\Software\Classes enquanto o objeto existir.
class CClassesRootRedirect
{
public:
    explicit CClassesRootRedirect ( HKEY root ) : m_hKey(NULL), m_lRet(ERROR_SUCCESS)
    {
        m_lRet = RegCreateKeyEx ( root, _T("Software\\Classes"), 0, NULL, 0,
                                  KEY_ALL_ACCESS, NULL, &m_hKey, NULL );
        if ( ERROR_SUCCESS == m_lRet )
            m_lRet = RegOverridePredefKey ( HKEY_CLASSES_ROOT, m_hKey );
    }
    ~CClassesRootRedirect()
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

// Lista pequena e sem repeticao de chaves base (".nds", ProgIDs).
struct CBindings
{
    enum { MAX = 8 };
    TCHAR path[MAX][MAX_PATH];
    int   count;

    CBindings() : count(0) { }

    void Add ( LPCTSTR p )
    {
        if ( NULL == p || 0 == p[0] || count >= MAX )
            return;
        for ( int i = 0; i < count; i++ )
            if ( 0 == lstrcmpi ( path[i], p ) )
                return;
        lstrcpyn ( path[count++], p, MAX_PATH );
    }
};

static bool ReadString ( HKEY root, LPCTSTR sub, LPCTSTR name, TCHAR* out, DWORD cch )
{
    DWORD cb = cch * sizeof(TCHAR);
    out[0] = 0;
    return ERROR_SUCCESS == RegGetValue ( root, sub, name, RRF_RT_REG_SZ, NULL, out, &cb )
        && 0 != out[0];
}

// ProgID que pode ser ligado ao handler.
//  - "Applications\x.exe" (gerado por "Abrir com -> Sempre") e "AppX..."
//    (apps UWP) valem para varias extensoes ao mesmo tempo; ligar o handler
//    neles afetaria .gba, .zip etc.
//  - Por maquina, um ProgID que existe em HKCU\Software\Classes seria
//    sombreado pela copia do HKCU (o DefaultIcon dela vence); esse caso e do
//    registro por usuario.
static bool IsBindableProgId ( LPCTSTR progId, bool perUser )
{
    if ( 0 == _tcsnicmp ( progId, _T("Applications\\"), 13 )
         || 0 == _tcsnicmp ( progId, _T("AppX"), 4 ) )
        return false;

    if ( !perUser )
        {
        TCHAR sub[MAX_PATH + 32];
        HKEY  h;
        _stprintf_s ( sub, _T("Software\\Classes\\%s"), progId );
        if ( ERROR_SUCCESS == RegOpenKeyEx ( HKEY_CURRENT_USER, sub, 0, KEY_QUERY_VALUE, &h ) )
            {
            RegCloseKey ( h );
            return false;
            }
        }

    return true;
}

// Quem e dono do .nds agora. Precisa rodar ANTES do redirecionamento,
// para enxergar a visao mesclada (HKLM + HKCU) do HKCR.
static void DiscoverBindings ( CBindings& b, bool perUser )
{
    TCHAR progId[MAX_PATH];

    b.Add ( _T(".nds") );
    if ( ReadString ( HKEY_CLASSES_ROOT, _T(".nds"), NULL, progId, MAX_PATH )
         && IsBindableProgId ( progId, perUser ) )
        b.Add ( progId );
    if ( ReadString ( HKEY_CURRENT_USER, USERCHOICE_KEY, _T("ProgId"), progId, MAX_PATH )
         && IsBindableProgId ( progId, perUser ) )
        b.Add ( progId );
}

// Valor bruto (qualquer tipo) com terminador garantido.
struct CRawValue
{
    BYTE  data[2 * MAX_PATH * sizeof(TCHAR) + sizeof(TCHAR)];
    DWORD type;
    DWORD size;

    bool Read ( HKEY key, LPCTSTR name )
    {
        ZeroMemory ( data, sizeof(data) );
        size = sizeof(data) - sizeof(TCHAR);
        return ERROR_SUCCESS == RegQueryValueEx ( key, name, NULL, &type, data, &size );
    }
    bool Equals ( LPCTSTR s ) const
    {
        return ( REG_SZ == type || REG_EXPAND_SZ == type )
            && 0 == lstrcmpi ( (LPCTSTR) data, s );
    }
    void Write ( HKEY key, LPCTSTR name ) const
    {
        RegSetValueEx ( key, name, 0, type, data, size );
    }
};

// Apaga a chave se nao tiver mais valores nem subchaves.
static void DeleteKeyIfEmpty ( LPCTSTR sub )
{
    HKEY  h;
    DWORD subKeys = 0, values = 0;

    if ( ERROR_SUCCESS != RegOpenKeyEx ( HKEY_CLASSES_ROOT, sub, 0, KEY_QUERY_VALUE, &h ) )
        return;
    LONG r = RegQueryInfoKey ( h, NULL, NULL, NULL, &subKeys, NULL, NULL, &values,
                               NULL, NULL, NULL, NULL );
    RegCloseKey ( h );

    if ( ERROR_SUCCESS == r && 0 == subKeys && 0 == values )
        RegDeleteKey ( HKEY_CLASSES_ROOT, sub );
}

// Define o valor default de <sub> como 'value', guardando o anterior em
// 'backupName' (uma vez so; nao guarda se o anterior ja era o nosso).
static LONG SetWithBackup ( LPCTSTR sub, LPCTSTR value, LPCTSTR backupName )
{
    CRegKey key;
    LONG r = key.Create ( HKEY_CLASSES_ROOT, sub );
    if ( ERROR_SUCCESS != r )
        return r;

    CRawValue cur, old;
    if ( cur.Read ( key.m_hKey, NULL ) && !cur.Equals ( value )
         && !old.Read ( key.m_hKey, backupName ) )
        cur.Write ( key.m_hKey, backupName );

    return key.SetStringValue ( NULL, value );
}

// Desfaz SetWithBackup: restaura o backup se houver um valido; senao, se o
// valor ainda for o nosso, remove-o (e a chave, se ficar vazia).
static void RestoreFromBackup ( LPCTSTR sub, LPCTSTR value, LPCTSTR backupName )
{
    CRegKey key;
    if ( ERROR_SUCCESS != key.Open ( HKEY_CLASSES_ROOT, sub, KEY_QUERY_VALUE | KEY_SET_VALUE ) )
        return;

    CRawValue cur, old;
    bool haveCur = cur.Read ( key.m_hKey, NULL );

    if ( old.Read ( key.m_hKey, backupName ) )
        {
        key.DeleteValue ( backupName );
        // Backup igual ao nosso valor e sobra de versao antiga: ignorar.
        if ( !old.Equals ( value ) )
            {
            if ( !haveCur || cur.Equals ( value ) )
                old.Write ( key.m_hKey, NULL );
            key.Close();
            return;
            }
        }

    if ( haveCur && cur.Equals ( value ) )
        key.DeleteValue ( NULL );

    key.Close();
    DeleteKeyIfEmpty ( sub );
}

static LONG Bind ( LPCTSTR base )
{
    TCHAR sub[MAX_PATH + 32];

    _stprintf_s ( sub, _T("%s\\DefaultIcon"), base );
    LONG r = SetWithBackup ( sub, _T("%1"), OLD_DEFICON );
    if ( ERROR_SUCCESS != r )
        return r;

    _stprintf_s ( sub, _T("%s\\ShellEx\\IconHandler"), base );
    r = SetWithBackup ( sub, CLSID_STR, OLD_ICONHANDLER );
    if ( ERROR_SUCCESS != r )
        return r;

    CRegKey key;
    r = key.Create ( HKEY_CLASSES_ROOT, BINDINGS_KEY );
    if ( ERROR_SUCCESS == r )
        r = key.SetStringValue ( base, _T("") );
    return r;
}

static void Unbind ( LPCTSTR base )
{
    TCHAR sub[MAX_PATH + 32];

    _stprintf_s ( sub, _T("%s\\ShellEx\\IconHandler"), base );
    RestoreFromBackup ( sub, CLSID_STR, OLD_ICONHANDLER );
    _stprintf_s ( sub, _T("%s\\ShellEx"), base );
    DeleteKeyIfEmpty ( sub );

    _stprintf_s ( sub, _T("%s\\DefaultIcon"), base );
    RestoreFromBackup ( sub, _T("%1"), OLD_DEFICON );

    DeleteKeyIfEmpty ( base );
}

// Chaves tocadas no registro anterior (lidas antes de apagar o CLSID).
static void ReadRecordedBindings ( CBindings& b )
{
    CRegKey key;
    if ( ERROR_SUCCESS != key.Open ( HKEY_CLASSES_ROOT, BINDINGS_KEY, KEY_QUERY_VALUE ) )
        return;

    TCHAR name[MAX_PATH];
    for ( DWORD i = 0; ; i++ )
        {
        DWORD cch = MAX_PATH;
        if ( ERROR_SUCCESS != RegEnumValue ( key.m_hKey, i, name, &cch, NULL, NULL, NULL, NULL ) )
            break;
        b.Add ( name );
        }
}

static HRESULT RegisterImpl ( bool perUser )
{
    CBindings bindings;
    DiscoverBindings ( bindings, perUser );

    if ( !perUser )
        {
        // "Approved": nome do valor = CLSID, dado = descricao. So tem efeito
        // com a policy EnforceShellExtensionSecurity, mas e o registro correto.
        CRegKey key;
        LONG lRet = key.Open ( HKEY_LOCAL_MACHINE, APPROVED_KEY, KEY_SET_VALUE );
        if ( ERROR_SUCCESS == lRet )
            lRet = key.SetStringValue ( CLSID_STR, _T("NDS file icon extension") );
        if ( ERROR_SUCCESS != lRet )
            return HRESULT_FROM_WIN32 ( lRet );
        }

    CClassesRootRedirect redirect ( perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE );
    if ( ERROR_SUCCESS != redirect.Result() )
        return HRESULT_FROM_WIN32 ( redirect.Result() );

    // Mantem os ProgIDs de registros anteriores: o ForceRemove do .rgs
    // apaga CLSID\{...}\Bindings, e sem isso um dono antigo do .nds
    // ficaria com o handler pendurado depois do unregister.
    ReadRecordedBindings ( bindings );

    HRESULT hr = _Module.RegisterServer ( FALSE );
    if ( FAILED(hr) )
        return hr;

    for ( int i = 0; i < bindings.count; i++ )
        {
        LONG r = Bind ( bindings.path[i] );
        if ( ERROR_SUCCESS != r )
            {
            // Desfaz tudo para nao deixar registro pela metade. Inclui os
            // bindings de registros anteriores: sem o CLSID eles ficariam
            // apontando para um handler inexistente.
            for ( int j = 0; j < bindings.count; j++ )
                Unbind ( bindings.path[j] );
            _Module.UnregisterServer ( FALSE );
            return HRESULT_FROM_WIN32 ( r );
            }
        }

    return S_OK;
}

static HRESULT UnregisterImpl ( bool perUser )
{
    if ( !perUser )
        {
        CRegKey key;
        if ( ERROR_SUCCESS == key.Open ( HKEY_LOCAL_MACHINE, APPROVED_KEY, KEY_SET_VALUE ) )
            key.DeleteValue ( CLSID_STR );
        }

    CClassesRootRedirect redirect ( perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE );
    if ( ERROR_SUCCESS != redirect.Result() )
        return HRESULT_FROM_WIN32 ( redirect.Result() );

    // ".nds" sempre, para limpar tambem registros de versoes antigas.
    CBindings bindings;
    bindings.Add ( _T(".nds") );
    ReadRecordedBindings ( bindings );

    for ( int i = 0; i < bindings.count; i++ )
        Unbind ( bindings.path[i] );

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
