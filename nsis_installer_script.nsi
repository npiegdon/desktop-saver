;
; This is an NSIS installer script for DesktopSaver
;

!include "MUI.nsh"
!include "x64.nsh"

!define VERSION 3.1

Name "DesktopSaver ${VERSION}"
OutFile "DesktopSaver-${VERSION}-installer.exe"
InstallDir $PROGRAMFILES\DesktopSaver
BrandingText " "

SetCompressor /SOLID lzma

!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_SMALLDESC

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\DesktopSaver "Install_Dir"

ComponentText "This will install DesktopSaver to your computer."
DirText "Choose a directory to install in to:"

!include WinMessages.nsh
 
Function CloseProgram 
  Exch $1
  Push $0
  loop:
    FindWindow $0 $1
    IntCmp $0 0 done
    SendMessage $0 ${WM_CLOSE} 0 0
    Sleep 100 
    Goto loop 
  done: 
  Pop $0 
  Pop $1
FunctionEnd

Function un.CloseProgram 
  Exch $1
  Push $0
  loop:
    FindWindow $0 $1
    IntCmp $0 0 done
    SendMessage $0 ${WM_CLOSE} 0 0
    Sleep 100 
    Goto loop 
  done: 
  Pop $0 
  Pop $1
FunctionEnd

Section "!DesktopSaver" MainApp
SectionIn RO

  ; stop the application if it's running (this only works on WinXP+)
  DetailPrint "Attempting to stop any old versions of DesktopSaver before installing..."
  Push "desktop_saver"
  Call CloseProgram  
  Sleep 1000

  SetOutPath $INSTDIR

  ${If} ${RunningX64}
    DetailPrint "Deploying 64-bit DesktopSaver."
    File x64\Release\DesktopSaver.exe
  ${Else}
    DetailPrint "Deploying 32-bit DesktopSaver."
    File Release\DesktopSaver.exe
  ${EndIf}

  File license.txt

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\DesktopSaver "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DesktopSaver" "DisplayName" "DesktopSaver (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DesktopSaver" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"

SectionEnd


Section "Start Menu Shortcuts" Shortcuts
  SetShellVarContext all
  CreateShortCut "$SMPROGRAMS\DesktopSaver.lnk" "$INSTDIR\DesktopSaver.exe" "" "$INSTDIR\DesktopSaver.exe" 0
SectionEnd

Section "Run After Install" RunPostInstall
  Exec '"$INSTDIR\DesktopSaver.exe"'
SectionEnd

Section /o "Always Run at Startup" AlwaysRun
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "DesktopSaver" '"$INSTDIR\DesktopSaver.exe"'
SectionEnd


UninstallText "This will uninstall DesktopSaver. Hit next to continue."
Section "Uninstall"

  ; stop the application if it's running (this only works on WinXP)
  DetailPrint "Attempting to stop DesktopSaver before uninstalling..."
  Push "desktop_saver"
  Call un.CloseProgram  
  Sleep 1000

  ; remove registry keys
  ;
  ; NOTE: this intentionally leaves the "Install_Dir"
  ; entry in HKLM\SOFTWARE\DesktopSaver in the event of
  ; subsequent reinstalls/upgrades
  ;
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DesktopSaver"
  DeleteRegKey HKCU SOFTWARE\DesktopSaver

  ; remove the "run at startup" entry
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "DesktopSaver"

  ; delete program files
  Delete $INSTDIR\readme.txt
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\DesktopSaver.exe

  ; remove Start Menu shortcuts
  SetShellVarContext all
  Delete $SMPROGRAMS\DesktopSaver.lnk

  ; delete left over icon history file
  Delete "$APPDATA\DesktopSaver\*.*"
  RmDir "$APPDATA\DesktopSaver"

  RMDir /r "$INSTDIR"

SectionEnd


LangString DESC_MainApp ${LANG_ENGLISH} "Installs main DesktopSaver application files."
LangString DESC_Shortcuts ${LANG_ENGLISH} "Adds a DesktopSaver Start Menu group to the 'All Programs' section of your Start Menu."
LangString DESC_RunPostInstall ${LANG_ENGLISH} "Starts DesktopSaver immediately after install is complete."
LangString DESC_AlwaysRun ${LANG_ENGLISH} "Causes DesktopSaver to run each time Windows starts."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${MainApp} $(DESC_MainApp)
  !insertmacro MUI_DESCRIPTION_TEXT ${Shortcuts} $(DESC_Shortcuts)
  !insertmacro MUI_DESCRIPTION_TEXT ${RunPostInstall} $(DESC_RunPostInstall)
  !insertmacro MUI_DESCRIPTION_TEXT ${AlwaysRun} $(DESC_AlwaysRun)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
