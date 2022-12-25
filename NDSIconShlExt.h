// NDSIconShlExt.h : Declaration of the CNDSIconShlExt

#ifndef __NDSICONSHLEXT_H_
#define __NDSICONSHLEXT_H_

/////////////////////////////////////////////////////////////////////////////
// CTxtIconShlExt

class ATL_NO_VTABLE CNDSIconShlExt :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CNDSIconShlExt, &CLSID_NDSIconShlExt>,
    public IPersistFile,
    public IExtractIcon
{
public:
    CNDSIconShlExt() : m_qwFileSize(0) { }

    BEGIN_COM_MAP(CNDSIconShlExt)
        COM_INTERFACE_ENTRY(IPersistFile)
        COM_INTERFACE_ENTRY(IExtractIcon)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_TXTICONSHLEXT)

public:
    // IPersistFile
    STDMETHODIMP GetClassID( CLSID* ) { return E_NOTIMPL; }
    STDMETHODIMP IsDirty() { return E_NOTIMPL; }
    STDMETHODIMP Save( LPCOLESTR, BOOL ) { return E_NOTIMPL; }
    STDMETHODIMP SaveCompleted( LPCOLESTR ) { return E_NOTIMPL; }
    STDMETHODIMP GetCurFile( LPOLESTR* ) { return E_NOTIMPL; }

    STDMETHODIMP Load( LPCOLESTR wszFile, DWORD )
    { 
		USES_CONVERSION;
		lstrcpyn ( m_szFilename, W2CT(wszFile), MAX_PATH );
		return S_OK;
    }

    // IExtractIcon
    STDMETHODIMP GetIconLocation( UINT uFlags, LPTSTR szIconFile, UINT cchMax,
                                  int* piIndex, UINT* pwFlags );
    STDMETHODIMP Extract( LPCTSTR pszFile, UINT nIconIndex, HICON* phiconLarge,
                          HICON* phiconSmall, UINT nIconSize );

protected:
    TCHAR     m_szFilename[MAX_PATH];   // Full path to the file in question.
    DWORDLONG m_qwFileSize;             // Used by extraction method 2.

	HICON getFromNDSFile();
};

#endif //__NDSICONSHLEXT_H_
