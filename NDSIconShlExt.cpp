// TxtIconShlExt.cpp : Implementation of CTxtIconShlExt

#include "stdafx.h"
#include "resource.h"
#include "NDSFileIcons.h"
#include "NDSIconShlExt.h"

//HICON getFromNDSFile(LPCTSTR pszFile);
HICON createIcon(PBYTE iconData, int iconSize);

/////////////////////////////////////////////////////////////////////////////
// CTxtIconShlExt

/*
********** METHOD 1 **********
This method returns a filename/index pair to the shell, telling it where the
icon is.  This is the easier way, but it requires you to keep track of the
resource IDs of the icons *and* keep them in the right order!
*/
/*STDMETHODIMP CNDSIconShlExt::GetIconLocation (
    UINT uFlags,  LPTSTR szIconFile, UINT cchMax,
    int* piIndex, UINT* pwFlags )
{
DWORD     dwFileSizeLo, dwFileSizeHi;
DWORDLONG qwSize;
HANDLE    hFile;

    // First, open the file and get its length.
    hFile = CreateFile ( m_szFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        return S_FALSE;                 // tell the shell to use a default icon

    dwFileSizeLo = GetFileSize ( hFile, &dwFileSizeHi );

    CloseHandle ( hFile );

    // Check that GetFileSize() succeeded.
    if ( (DWORD) -1 == dwFileSizeLo && GetLastError() != NO_ERROR )
        return S_FALSE;                 // tell the shell to use a default icon

    // The icons are all in this DLL, so get the full path to the DLL, which
    // we'll return through the szIconFile parameter.
TCHAR szModulePath[MAX_PATH];

    GetModuleFileName ( _Module.GetResourceInstance(), szModulePath, MAX_PATH );
    lstrcpyn ( szIconFile, szModulePath, cchMax );

    // Decide which icon to use based on the file size.
    qwSize = DWORDLONG(dwFileSizeHi)<<32 | dwFileSizeLo;

    if ( 0 == qwSize )
        *piIndex = 0;
    else if ( qwSize < 4096 )
        *piIndex = 1;
    else if ( qwSize < 8192 )
        *piIndex = 2;
    else 
        *piIndex = 3;

	*piIndex = 3;

    // pwFlags is set to zero to get the default behavior from Explorer.  You 
    // can set it to GIL_SIMULATEDOC to have Explorer put the icon we return in
    // a "dog-eared paper" icon, and use _that_ as the icon for the file.
    //*pwFlags = GIL_SIMULATEDOC;
    *pwFlags = 0;

    return S_OK;
}

STDMETHODIMP CNDSIconShlExt::Extract (
    LPCTSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall,
    UINT nIconSize )
{
    return S_FALSE;                     // Tell the shell to do the extracting itself.
}*/

/*
********** METHOD 2 **********
This method stores the file size in the CTxtIconShlExt object, and does the
icon extraction manually.  This way is more complicated, but I prefer it since
you don't have to do bookkeeping of your resource IDs.
*/

STDMETHODIMP CNDSIconShlExt::GetIconLocation (
    UINT uFlags,  LPTSTR szIconFile, UINT cchMax,
    int* piIndex, UINT* pwFlags )
{
DWORD  dwFileSizeLo, dwFileSizeHi;
HANDLE hFile;

    hFile = CreateFile ( m_szFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        return S_FALSE;                 // tell the shell to use a default icon

    dwFileSizeLo = GetFileSize ( hFile, &dwFileSizeHi );

    CloseHandle ( hFile );

    if ( (DWORD) -1 == dwFileSizeLo && GetLastError() != NO_ERROR )
        return S_FALSE;                 // tell the shell to use a default icon

    m_qwFileSize = ((DWORDLONG) dwFileSizeHi)<<32 | dwFileSizeLo;

    // Tell the shell not to look at the name/index, and don't check the icon
    // cache.  Note that the GIL_NOTFILENAME flag doesn't really have any effect;
    // if we fill in szIconFile/piIndex, Explorer will still check them against
    // the cache.  So the only flag that's really important is GIL_DONTCACHE.
    *pwFlags = GIL_NOTFILENAME | GIL_DONTCACHE;

    return S_OK;
}

STDMETHODIMP CNDSIconShlExt::Extract (
    LPCTSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall,
    UINT nIconSize )
{
UINT uIconID;
WORD wSmallIconSize = HIWORD(nIconSize), wLargeIconSize = LOWORD(nIconSize);

    // Determine which icon to use, depending on the file size.
    /*if ( 0 == m_qwFileSize )
        uIconID = IDI_ZERO_BYTES;
    else if ( m_qwFileSize < 4096 )
        uIconID = IDI_UNDER_4K;
    else if ( m_qwFileSize < 8192 )
        uIconID = IDI_UNDER_8K;
    else 
        uIconID = IDI_OVER_8K;*/

	uIconID = IDI_NDS;
    // Load the icons!  The docs say that either HICON* param may be NULL,
    // so we have to check for that case.
    
	/*if ( NULL != phiconLarge )
        {
        *phiconLarge = (HICON) LoadImage ( _Module.GetResourceInstance(),
                                           MAKEINTRESOURCE(uIconID), IMAGE_ICON,
                                           wLargeIconSize, wLargeIconSize,
                                           LR_DEFAULTCOLOR );
        }

    if ( NULL != phiconSmall )
        {
        *phiconSmall = (HICON) LoadImage ( _Module.GetResourceInstance(),
                                           MAKEINTRESOURCE(uIconID), IMAGE_ICON,
                                           wSmallIconSize, wSmallIconSize,
                                           LR_DEFAULTCOLOR );
        }*/

	if ( NULL != phiconLarge ) {
		*phiconLarge = getFromNDSFile();
	}


	if (NULL != phiconSmall) {
		*phiconSmall = getFromNDSFile();
	}
    return S_OK;
}

/*#define BUFFERSIZE 81
HICON getFromNDSFile2(LPCTSTR pszFile)
{
	int size = 1024;
	DWORD dwBytesRead = 0;
    char ReadBuffer[BUFFERSIZE] = {0};
	
	//PBYTE pData = new BYTE[size];
	//if (NULL == pdata) 
	//	return NULL;
	HANDLE hFile;

	hFile = CreateFile(pszFile,                // name of the write
		GENERIC_READ,          // open for writing
        FILE_SHARE_READ,       // share it
        NULL,                   // default security
        OPEN_EXISTING,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
    NULL); 

	if (INVALID_HANDLE_VALUE == hFile) {
		return NULL;
	}

	DWORD dwOffset = SetFilePointer(hFile, 
		lDistance, 
		NULL, 
    FILE_BEGIN); 

	if (dwPtr == INVALID_SET_FILE_POINTER) {
		//DWORD dwError = GetLastError() ;        
		CloseHandle(hFile);
        return NULL;
	}				

	if( FALSE == ReadFile(hFile, ReadBuffer, BUFFERSIZE-1, &dwBytesRead, NULL) )
    {
        //DisplayError(TEXT("ReadFile"));
        //printf("Terminal failure: Unable to read from file.\n");
        CloseHandle(hFile);
        return NULL;
    }

	CloseHandle(hFile);
}*/

HICON CNDSIconShlExt::getFromNDSFile()
{	
	FILE *fp;
	UINT addr[1];
	UCHAR tiles[512];
	UCHAR palette[32];
	DWORD colors[16];
	BYTE data[4286];

	//MessageBox(NULL, m_szFilename, "Teste", 0);

	fopen_s(&fp, m_szFilename, "rb");
	if (NULL == fp) {
		MessageBox(NULL, "Error", "Error", 0);
	}

	fseek(fp, 0x68, SEEK_SET);
    fread(addr, sizeof(int), 1, fp);

	//tiles
    fseek(fp, addr[0] + 0x20, SEEK_SET);
    fread(tiles, 1, 0x200, fp);
	fread(palette, 1, 0x20, fp);
	
	int idx = 0;
    for (int i = 0; i < 32; i++) {
        unsigned char c1, c2;
        c1 = palette[i++];
        c2 = palette[i];
        unsigned int color = (c2 << 8) | c1;
        //unsigned int color =  c1 + (c2 << 8);
    
        unsigned int alpha = (color & 0x8000);
        unsigned int R = (color & 0x1f);			//Calcula nivel do vermelho
    	unsigned int G = ((color >> 5) & 0x1f);		//Calcula nivel do verde
    	unsigned int B = ((color >> 10) & 0x1f);
    
        //printf("%d %d %d\r\n", c1, c2, color);
        //printf("RGB:%u %u %u %u\r\n", R, G, B, alpha);
        R = (int) (8.25 * R);
        G = (int) (8.25 * G);
        B = (int) (8.25 * B);        
        //R = R << 3; G = G << 3; B = B << 3;
        //R = (int)(R * 255/31); 
        //G = (int)(G * 255/31);
        //B = (int)(B * 255/31);
        //printf("RGB:%u %u %u\r\n", R, G, B);
        
        //color = (1 << 32) | (R << 16) | (G << 8) | B;
        color = (255 << 24) | (R << 16) | (G << 8) | B;
        colors[idx++] = color;
        //printf("%u\r\n", color);
        //i++;
    }
	
	//monta o icone no braço
	//melhorar isso aqui depois
	data[0] = 0x00; data[1] = 0x00;
	data[2] = 0x01; data[3] = 0x00;
	data[4] = 0x01; data[5] = 0x00;

	data[6] = 0x20; //W
	data[7] = 0x20; //H
	data[8] = data[9] = 0x00; //reservado

	data[10] = 0x01; data[11] = 0x00; //bit planes
	data[12] = 0x20; data[13] = 0x00; //bit per pixel
	
	data[14] = 0xA8; data[15] = 0x10; data[16] = 0x00; data[17] = 0x00; //raw data
	data[18] = 0x16; data[19] = 0x00; data[20] = 0x00; data[21] = 0x00; //offset of bitmap (22)

	
	data[22] = 0x28; data[23] = 0x00; data[24] = 0x00; data[25] = 0x00; //bitmap header
	data[26] = 0x20; data[27] = 0x00; data[28] = 0x00; data[29] = 0x00; //bitmap Width
	data[30] = 0x40; data[31] = 0x00; data[32] = 0x00; data[33] = 0x00; //bitmap double Height

	data[34] = 0x01; data[35] = 0x00; //color planes
	data[36] = 0x20; data[37] = 0x00; //bit per pixel

	data[38] = 0x00; data[39] = 0x00; data[40] = 0x00; data[41] = 0x00; //compression mode
	data[42] = 0x00; data[43] = 0x10; data[44] = 0x00; data[45] = 0x00; //size of bitmap data

	data[46] = 0x00; data[47] = 0x00; data[48] = 0x00; data[49] = 0x00; //hor res
	data[50] = 0x00; data[51] = 0x00; data[52] = 0x00; data[53] = 0x00; //ver res
	data[54] = 0x00; data[55] = 0x00; data[56] = 0x00; data[57] = 0x00; //color in pal
	data[58] = 0x00; data[59] = 0x00; data[60] = 0x00; data[61] = 0x00; //important


	int tile = 0, transparent = 0, offset = 62, tmp;
	DWORD color;
    for (int ntiles = 0; ntiles < 16; ntiles++) {
        for (int k = ntiles / 4 * 8; k < (ntiles / 4 * 8) + 8; k++) {
            for (int l = ntiles % 4 * 8; l < (ntiles % 4 * 8) + 8; l++) {
                unsigned int col = 0;
                //printf("%d %d %d\r\n", k, l, tiles[tile]);
                if ((l % 2) == 1) {
                    if (tiles[tile] > 1 && tile < 512) {
                       col = (tiles[tile] << 8) | tiles[tile+1];
                       col &= 0xf00;                       
                       col >>= 8;
                    }
                    tile++;  
                    //bitmap[31-k][l-1] = colors[col];
					color = colors[col];
					tmp = ((31-k) * 32) + (l-1);
					tmp *= 4;
					//tmp = (31 - k) * (l - 1);
					DWORD val = (color & 0xff000000) >> 24;
					data[tmp + offset + 3] = (BYTE) ((color & 0xff000000) >> 24);
					val = (color & 0x00ff0000) >> 16;
					data[tmp + offset + 2] = (BYTE) ((color & 0x00ff0000) >> 16);
					val = (color & 0x0000ff00) >> 8;
					data[tmp + offset + 1] = (BYTE) ((color & 0x0000ff00) >> 8);
					val = color & 0x000000ff;
					data[tmp + offset + 0] = (BYTE) (color & 0x000000ff);
					//offset += 4;
                } else {
                    if (tile < 512) {
                       col = (tiles[tile] << 8) | tiles[tile+1];
                       col &= 0xf000;
                       col >>= 12;
                    }
                    //bitmap[31-k][l+1] = colors[col];
					color = colors[col];
					tmp = ((31-k) * 32) + (l+1);
					tmp *= 4;
					DWORD val = (color &0xff000000) >> 24;
					data[tmp + offset + 3] = (BYTE) ((color &0xff000000) >> 24);
					val = (color & 0x00ff0000) >> 16;
					data[tmp + offset + 2] = (BYTE) ((color & 0x00ff0000) >> 16);
					val = (color & 0x0000ff00) >> 8;
					data[tmp + offset + 1] = (BYTE) ((color & 0x0000ff00) >> 8);
					val = color & 0x000000ff;
					data[tmp + offset + 0] = (BYTE) (color & 0x000000ff);
					//offset += 4;
                }
                //bitmap[31 - k][l] = colors[col];                
                //printf("%d %d %d %d %d\r\n", ntiles, k, l, col, colors[col]);
                if (l == 4 && k == 0) {
                  transparent = colors[col];                  
                  //printf("transparente: %d\r\n", colors[col]);
                }
            }                
        }
    }
	
	offset = 4158;
	for (int i = 0; i < 128; i++) {
		data[offset + i] = 0x00;
	}

	fclose(fp);

	/*FILE *fp2;
	fopen_s(&fp2, "teste.ico", "w");
	fwrite((PBYTE) data, sizeof(BYTE), 4285, fp2);
	fclose(fp2);*/

	HANDLE hFile; 
	hFile = CreateFile("c:\\teste.ico",                // name of the write
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       CREATE_ALWAYS,             // create new file only
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) 
    { 
		MessageBox(NULL, "Erro ao abrir", "Teste", 0);
        //DisplayError(TEXT("CreateFile"));
        //_tprintf(TEXT("Terminal failure: Unable to open file \"%s\" for write.\n"), argv[1]);
        //return;
    }

	DWORD dwBytesToWrite = 4286;
    DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = WriteFile( 
                    hFile,           // open file handle
                    data,      // start of data to write
                    dwBytesToWrite,  // number of bytes to write
                    &dwBytesWritten, // number of bytes that were written
                    NULL);            // no overlapped structure

	if (FALSE == bErrorFlag)
    {
		MessageBox(NULL, "Error", "Unable", 0);
        //DisplayError(TEXT("WriteFile"));
        //printf("Terminal failure: Unable to write to file.\n");
    }
    else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            // This is an error because a synchronous write that results in
            // success (WriteFile returns TRUE) should write all data as
            // requested. This would not necessarily be the case for
            // asynchronous writes.
            //printf("Error: dwBytesWritten != dwBytesToWrite\n");
			MessageBox(NULL, "Error: dwBytesWritten != dwBytesToWrite", "Teste", 0);
        }
        else
        {
            //_tprintf(TEXT("Wrote %d bytes to %s successfully.\n"), dwBytesWritten, argv[1]);
			//Error: dwBytesWritten != dwBytesToWrite
        }
    }

    CloseHandle(hFile);

	HICON hIcon = createIcon(data, 32);
	if (hIcon == NULL)
		MessageBox(NULL, "NULL", "Teste", 0);
	return hIcon;
}

HICON createIcon(PBYTE iconData, int iconSize)     
{     
   HICON hIcon = NULL;
   // Ahhh, this is the magic API.     
   int offset = LookupIconIdFromDirectoryEx(iconData, TRUE, iconSize, iconSize, LR_DEFAULTCOLOR);     

   if (offset != 0) {
      hIcon = CreateIconFromResourceEx(iconData + offset, 0, TRUE, 0x30000, iconSize, iconSize, LR_DEFAULTCOLOR);     
   } else {
		MessageBox(NULL, "Offset == 0", "Teste", 0);
   }

   //MessageBox(NULL, "Icon", "Teste", 0);
   return hIcon;     
}
