# NDSShellExtension
Shell Extension to show Nintendo DS rom icon in Explorer.

It reads the icon stored in the banner of each `.nds` file (32x32, 4bpp,
16-color palette) and shows it as the file icon in Windows Explorer.

## Requirements

- Windows 10/11 **64-bit**. The DLL must be x64: the 64-bit Explorer does not
  load 32-bit shell extensions.
- To build: Visual Studio 2022 (or Build Tools) with the
  *Desktop development with C++* workload and the **C++ ATL** component.

## Build

Open `NDSFileIcons.sln` and build `Release | x64`, or from a command line:

```
msbuild NDSFileIcons.sln /p:Configuration=Release /p:Platform=x64
```

Output: `x64\ReleaseU\NDSFileIcons.dll`.

## Install

Copy the DLL to a permanent folder and register it:

```
regsvr32 C:\Tools\NDSFileIcons.dll
```

- From an elevated prompt it registers for all users (HKLM).
- Without admin rights it registers for the current user only
  (`HKCU\Software\Classes`), no error.
- `regsvr32 /n /i:user C:\Tools\NDSFileIcons.dll` forces per-user
  registration.

Restart Explorer to pick it up:

```
taskkill /f /im explorer.exe & start explorer
```

If the generic icon still shows, clear the icon cache with `ie4uinit.exe -show`.

## Uninstall

```
regsvr32 /u C:\Tools\NDSFileIcons.dll
```

Removes the per-user registration, and the machine-wide one when run elevated.

## Test file

`ok.nds` is a minimal synthetic ROM (header + banner only). Its icon is a 4x4
grid of 8x8 tiles: red (top row, 2nd tile), green (2nd row, 2nd tile), blue
(bottom-right), transparent first column, everything else black.

## Troubleshooting

- Check the registration landed in the 64-bit view:
  `reg query "HKCR\CLSID\{DF4F5AE4-E597-4C12-BC26-2776C27F71AE}\InprocServer32"`.
  If it only exists under `WOW6432Node`, a 32-bit DLL was registered.
- If an emulator owns `.nds` (a ProgID in `HKCR\.nds` or a `UserChoice` under
  `HKCU\...\Explorer\FileExts\.nds`), that ProgID's `DefaultIcon` may win over
  this handler.
- An elevated Explorer ignores per-user (HKCU) registrations.

## History

Project from 2011, last 32-bit version from 2012. Updated in 2026 for
Windows 11: x64 build, VS 2022 toolset, per-user registration, debug code
removed (`c:\teste.ico`, `MessageBox`) and the icon is now built directly as a
32bpp DIB at the size Explorer asks for, with palette color 0 transparent.

Based on http://www.codeproject.com/shell/ShellExtGuide9.asp
written by:
  Michael Dunn (acidhelm@gmail.com)
Release date:
  June 3, 2006
