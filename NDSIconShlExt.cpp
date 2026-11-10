// NDSIconShlExt.cpp : Implementation of CNDSIconShlExt

#include "stdafx.h"
#include "resource.h"
#include "NDSFileIcons.h"
#include "NDSIconShlExt.h"

#include <stdio.h>
#include <tchar.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

// Banner da ROM (ver GBATEK, "DS Cartridge Icon/Title"):
//   header+0x68 : offset do banner (0 = sem banner)
//   banner+0x00 : versao (0x0001, 0x0002, 0x0003 ou 0x0103)
//   banner+0x20 : bitmap 32x32 4bpp, 4x4 tiles de 8x8 (512 bytes)
//   banner+0x220: paleta, 16 cores BGR555 (32 bytes); cor 0 = transparente
//   banner v1 tem 0x840 bytes (as versoes seguintes sao maiores)
static const long    BANNER_PTR_OFFSET  = 0x68;
static const long    BANNER_ICON_OFFSET = 0x20;
static const __int64 HEADER_SIZE        = 0x200;
static const __int64 BANNER_V1_SIZE     = 0x840;
static const int     NDS_ICON_SIZE      = 32;

static HICON createIcon ( const DWORD* pixels, int size );

/////////////////////////////////////////////////////////////////////////////
// CNDSIconShlExt

STDMETHODIMP CNDSIconShlExt::GetIconLocation (
    UINT uFlags,  LPTSTR szIconFile, UINT cchMax,
    int* piIndex, UINT* pwFlags )
{
    // Nao deixar lixo nos parametros de saida.
    if ( NULL != szIconFile && cchMax > 0 )
        szIconFile[0] = _T('\0');
    if ( NULL != piIndex )
        *piIndex = 0;
    *pwFlags = 0;

    // Le o arquivo uma vez so; o Extract usa o resultado guardado.
    m_haveIcon = readNDSIcon ( m_pixels );

    if ( m_haveIcon )
        {
        // Icone gerado por arquivo: sem nome/indice e sem cache.
        *pwFlags = GIL_NOTFILENAME | GIL_DONTCACHE;
        return S_OK;
        }

    // Nao e uma ROM valida (ou o ProgID e compartilhado com outra extensao):
    // devolve o icone que o ProgID tinha antes do registro, para o shell
    // extrair ele mesmo. Sem backup, S_FALSE = icone padrao.
    if ( NULL != szIconFile && NULL != piIndex
         && getFallbackIcon ( szIconFile, cchMax, piIndex ) )
        return S_OK;

    return S_FALSE;
}

STDMETHODIMP CNDSIconShlExt::Extract (
    LPCTSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall,
    UINT nIconSize )
{
int largeSize = LOWORD(nIconSize), smallSize = HIWORD(nIconSize);

    if ( NULL != phiconLarge ) *phiconLarge = NULL;
    if ( NULL != phiconSmall ) *phiconSmall = NULL;

    // Sem icone proprio (fallback ou erro): o shell extrai pela localizacao.
    if ( !m_haveIcon )
        return S_FALSE;

    if ( 0 == largeSize ) largeSize = GetSystemMetrics ( SM_CXICON );
    if ( 0 == smallSize ) smallSize = GetSystemMetrics ( SM_CXSMICON );

    if ( NULL != phiconLarge )
        {
        *phiconLarge = createIcon ( m_pixels, largeSize );
        if ( NULL == *phiconLarge )
            return S_FALSE;
        }

    if ( NULL != phiconSmall )
        *phiconSmall = createIcon ( m_pixels, smallSize );

    return S_OK;
}

// Icone original do ProgID associado a extensao do arquivo, guardado em
// DefaultIcon\OldDefIcon pelo registro. Formato "arquivo,indice".
bool CNDSIconShlExt::getFallbackIcon ( LPTSTR szIconFile, UINT cchMax, int* piIndex )
{
    HKEY hk = NULL;
    if ( FAILED ( AssocQueryKey ( ASSOCF_NONE, ASSOCKEY_CLASS,
                                  PathFindExtension ( m_szFilename ), NULL, &hk ) ) )
        return false;

    TCHAR loc[MAX_PATH];
    DWORD cb = sizeof(loc);
    LONG  r  = RegGetValue ( hk, _T("DefaultIcon"), _T("OldDefIcon"),
                             RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, NULL, loc, &cb );
    RegCloseKey ( hk );

    if ( ERROR_SUCCESS != r || 0 == loc[0] || 0 == lstrcmpi ( loc, _T("%1") ) )
        return false;

    *piIndex = PathParseIconLocation ( loc );
    lstrcpyn ( szIconFile, loc, cchMax );
    return 0 != szIconFile[0];
}

// Le o icone do banner da ROM e decodifica para 32x32 ARGB (top-down)
// em 'pixels'. Retorna false se o arquivo nao abrir ou nao for uma ROM com
// banner valido (protege contra arquivos de outro tipo que usem o mesmo
// ProgID).
bool CNDSIconShlExt::readNDSIcon ( DWORD* pixels )
{
FILE*  fp = NULL;
UINT   bannerOffset = 0;
WORD   version = 0;
BYTE   tiles[NDS_ICON_SIZE * NDS_ICON_SIZE / 2];
BYTE   palette[32];
DWORD  colors[16];

    if ( 0 != _tfopen_s ( &fp, m_szFilename, _T("rb") ) || NULL == fp )
        return false;

    __int64 fileSize = -1;
    if ( 0 == _fseeki64 ( fp, 0, SEEK_END ) )
        fileSize = _ftelli64 ( fp );

    bool ok = fileSize >= HEADER_SIZE
           && 0 == _fseeki64 ( fp, BANNER_PTR_OFFSET, SEEK_SET )
           && 1 == fread ( &bannerOffset, sizeof(bannerOffset), 1, fp )
           && (__int64) bannerOffset >= HEADER_SIZE
           && (__int64) bannerOffset + BANNER_V1_SIZE <= fileSize
           && 0 == _fseeki64 ( fp, bannerOffset, SEEK_SET )
           && 1 == fread ( &version, sizeof(version), 1, fp )
           && ( 0x0001 == version || 0x0002 == version
             || 0x0003 == version || 0x0103 == version )
           && 0 == _fseeki64 ( fp, (__int64) bannerOffset + BANNER_ICON_OFFSET, SEEK_SET )
           && sizeof(tiles)   == fread ( tiles,   1, sizeof(tiles),   fp )
           && sizeof(palette) == fread ( palette, 1, sizeof(palette), fp );

    fclose ( fp );

    if ( !ok )
        return false;

    // Paleta BGR555 -> ARGB. Expande 5 bits para 8 replicando os bits
    // altos (31 -> 255). A cor 0 e transparente no DS.
    for ( int i = 0; i < 16; i++ )
        {
        unsigned int c = palette[2*i] | (palette[2*i+1] << 8);
        unsigned int r =  c        & 0x1f;
        unsigned int g = (c >> 5)  & 0x1f;
        unsigned int b = (c >> 10) & 0x1f;

        r = (r << 3) | (r >> 2);
        g = (g << 3) | (g >> 2);
        b = (b << 3) | (b >> 2);

        colors[i] = ( i == 0 ) ? 0 : (0xFF000000u | (r << 16) | (g << 8) | b);
        }

    // 4x4 tiles de 8x8, 32 bytes cada, linha a linha dentro do tile.
    // Cada byte = 2 pixels: nibble baixo = esquerda, nibble alto = direita.
    const BYTE* src = tiles;
    for ( int tile = 0; tile < 16; tile++ )
        {
        int baseX = (tile % 4) * 8;
        int baseY = (tile / 4) * 8;

        for ( int y = baseY; y < baseY + 8; y++ )
            {
            for ( int x = baseX; x < baseX + 8; x += 2, src++ )
                {
                pixels[y * NDS_ICON_SIZE + x]     = colors[*src & 0x0f];
                pixels[y * NDS_ICON_SIZE + x + 1] = colors[*src >> 4];
                }
            }
        }

    return true;
}

// Cria um HICON size x size a partir do bitmap 32x32 ARGB, ampliando ou
// reduzindo por vizinho mais proximo (mantem o pixel art nitido).
static HICON createIcon ( const DWORD* pixels, int size )
{
    if ( size <= 0 || size > 256 )
        return NULL;

    BITMAPV5HEADER bi = { 0 };
    bi.bV5Size        = sizeof(bi);
    bi.bV5Width       = size;
    bi.bV5Height      = -size;          // top-down
    bi.bV5Planes      = 1;
    bi.bV5BitCount    = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask     = 0x00FF0000;
    bi.bV5GreenMask   = 0x0000FF00;
    bi.bV5BlueMask    = 0x000000FF;
    bi.bV5AlphaMask   = 0xFF000000;

    DWORD*  bits  = NULL;
    HBITMAP color = CreateDIBSection ( NULL, (BITMAPINFO*) &bi, DIB_RGB_COLORS,
                                       (void**) &bits, NULL, 0 );
    if ( NULL == color )
        return NULL;

    for ( int y = 0; y < size; y++ )
        {
        const DWORD* row = pixels + (y * NDS_ICON_SIZE / size) * NDS_ICON_SIZE;
        for ( int x = 0; x < size; x++ )
            bits[y * size + x] = row[x * NDS_ICON_SIZE / size];
        }

    // Mascara AND coerente com o alfa: bit 1 = transparente. Com 32bpp o
    // Windows usa o alfa, mas se o alfa for todo 0 (icone todo transparente)
    // ele cai para a mascara, entao ela precisa estar certa. Linhas de
    // bitmap monocromatico sao alinhadas em WORD.
    // Maximo 256x256: 32 bytes por linha * 256 = 8 KB, cabe na pilha.
    BYTE maskBits[((256 + 15) / 16) * 2 * 256];
    int  stride = ((size + 15) / 16) * 2;
    ZeroMemory ( maskBits, stride * size );

    for ( int y = 0; y < size; y++ )
        for ( int x = 0; x < size; x++ )
            if ( 0 == (bits[y * size + x] >> 24) )
                maskBits[y * stride + x / 8] |= (BYTE) (0x80 >> (x % 8));

    HBITMAP mask = CreateBitmap ( size, size, 1, 1, maskBits );

    HICON hIcon = NULL;
    if ( NULL != mask )
        {
        ICONINFO ii = { 0 };
        ii.fIcon    = TRUE;
        ii.hbmColor = color;
        ii.hbmMask  = mask;
        hIcon = CreateIconIndirect ( &ii );
        DeleteObject ( mask );
        }

    DeleteObject ( color );
    return hIcon;
}
