; Script Installer NSIS untuk Rapid Texter
; Dibuat untuk project aleafarrel-id/rapid-texter

;--------------------------------
; Include Modern UI (Agar tampilan installer terlihat modern)
  !include "MUI2.nsh"

;--------------------------------
; Konfigurasi Umum

  ; Nama aplikasi
  Name "Rapid Texter"
  
  ; Nama file installer yang akan dihasilkan
  OutFile "RapidTexter_Setup.exe"

  ; Folder default instalasi (Program Files)
  ; Gunakan $PROGRAMFILES64 jika aplikasi Anda build 64-bit
  InstallDir "$PROGRAMFILES64\Rapid Texter"
  
  ; Menyimpan lokasi install di registry (agar installer ingat lokasi install sebelumnya)
  InstallDirRegKey HKCU "Software\Rapid Texter" ""

  ; Request privileges untuk Windows Vista/7/10/11 (Admin rights)
  RequestExecutionLevel admin

;--------------------------------
; Konfigurasi Interface & Logo

  ; Menggunakan icon dari folder resources
  !define MUI_ICON "resources\app_icon.ico" 
  !define MUI_UNICON "resources\app_icon.ico"

  ; Tampilkan peringatan jika user membatalkan install
  !define MUI_ABORTWARNING

;--------------------------------
; Halaman Installer (Pages)

  ; Halaman Selamat Datang
  !insertmacro MUI_PAGE_WELCOME
  ; Halaman Lisensi (Menggunakan file LICENSE Anda)
  !insertmacro MUI_PAGE_LICENSE "LICENSE"
  ; Halaman Pilih Folder
  !insertmacro MUI_PAGE_DIRECTORY
  ; Halaman Proses Install
  !insertmacro MUI_PAGE_INSTFILES
  ; Halaman Selesai
  !insertmacro MUI_PAGE_FINISH

  ; Halaman Uninstaller
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Bahasa
  !insertmacro MUI_LANGUAGE "English" ; Bahasa Installer (bisa diganti "Indonesian" jika NSIS support full)

;--------------------------------
; BAGIAN INSTALASI (Section Utama)

Section "Rapid Texter Core" Sec01

  ; Set output path ke folder instalasi
  SetOutPath "$INSTDIR"
  
  ; --- FILE EXECUTABLE ---
  ; Ubah path di bawah ini agar sesuai lokasi hasil compile .exe.
  ; Misalnya: "build\Release\RapidTexter.exe" atau "bin\RapidTexter.exe"
  ; Saat ini diasumsikan file .exe ada di folder yang sama dengan script NSI.
  File "RapidTexter.exe" 
  
  ; --- MENYERTAKAN DEPENDENCY FOLDER ---
  
  ; 1. Copy Folder assets (Bahasa, dll)
  SetOutPath "$INSTDIR\assets"
  File /r "assets\*.*"
  
  ; 2. Copy Folder roll (Easter eggs/Media)
  SetOutPath "$INSTDIR\roll"
  File /r "roll\*.*"

  ; 3. Copy Folder resources (Icon, dll - Opsional tapi lebih baik untuk disertakan)
  SetOutPath "$INSTDIR\resources"
  File /r "resources\*.*"

  ; Kembali ke root install dir untuk membuat uninstaller
  SetOutPath "$INSTDIR"

  ; Buat Uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Simpan informasi di Registry (untuk Add/Remove Programs Windows)
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\RapidTexter" "DisplayName" "Rapid Texter"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\RapidTexter" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\RapidTexter" "DisplayIcon" "$INSTDIR\resources\app_icon.ico"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\RapidTexter" "Publisher" "Alea Farrel"

SectionEnd

;--------------------------------
; BAGIAN SHORTCUT (Start Menu & Desktop)

Section "Start Menu & Desktop Shortcuts" Sec02
  
  ; Buat Shortcut di Start Menu
  CreateDirectory "$SMPROGRAMS\Rapid Texter"
  CreateShortcut "$SMPROGRAMS\Rapid Texter\Rapid Texter.lnk" "$INSTDIR\RapidTexter.exe" "" "$INSTDIR\resources\app_icon.ico" 0
  CreateShortcut "$SMPROGRAMS\Rapid Texter\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\resources\app_icon.ico" 0
  
  ; Buat Shortcut di Desktop
  CreateShortcut "$DESKTOP\Rapid Texter.lnk" "$INSTDIR\RapidTexter.exe" "" "$INSTDIR\resources\app_icon.ico" 0

SectionEnd

;--------------------------------
; BAGIAN UNINSTALLER

Section "Uninstall"

  ; Hapus file executable
  Delete "$INSTDIR\RapidTexter.exe"
  Delete "$INSTDIR\Uninstall.exe"

  ; Hapus folder dependencies (Gunakan /r untuk recursive)
  RMDir /r "$INSTDIR\assets"
  RMDir /r "$INSTDIR\roll"
  RMDir /r "$INSTDIR\resources"

  ; Hapus folder instalasi utama
  RMDir "$INSTDIR"

  ; Hapus Shortcuts
  Delete "$SMPROGRAMS\Rapid Texter\Rapid Texter.lnk"
  Delete "$SMPROGRAMS\Rapid Texter\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Rapid Texter"
  Delete "$DESKTOP\Rapid Texter.lnk"

  ; Hapus Registry key
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\RapidTexter"
  DeleteRegKey HKCU "Software\Rapid Texter"

SectionEnd