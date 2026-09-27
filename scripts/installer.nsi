; EOL CAN Tool 安装器(NSIS 3.x)
; 特性:关闭运行中的程序 → 覆盖安装 → 开始菜单/桌面快捷方式 → 卸载器
; 用法:makensis /DVERSION=1.4.6 /DSRC=<nsis_pkg绝对路径> installer.nsi
;       产物 dist\EOL_CAN_Tool_Setup_v<VERSION>.exe
; 静默升级:/S(全自动,配合 QSimpleUpdater 下载后静默安装)

!include "MUI2.nsh"
!include "FileFunc.nsh"

!ifndef VERSION
  !define VERSION "1.5.0"
!endif
!ifndef SRC
  !define SRC "..\nsis_pkg"
!endif

Name "EOL CAN Tool v${VERSION}"
OutFile "..\dist\EOL_CAN_Tool_Setup_v${VERSION}.exe"
; per-user 安装:无需管理员,UAC 不打扰,静默升级(/S)可全自动
; 路径与原 QtIFW 安装位置一致,保证旧版可就地升级、QSimpleUpdater 无需改查找路径
InstallDir "$LOCALAPPDATA\EOL_CAN_Tool"
InstallDirRegKey HKCU "Software\EOL_CAN_Tool" "InstallDir"
RequestExecutionLevel user
Unicode true
SetCompressor /SOLID lzma
CRCCheck on
XPStyle on

!define MUI_ICON "..\resource\icons\exe_icon.ico"
!define MUI_UNICON "..\resource\icons\exe_icon.ico"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "SimpChinese"

; 卸载运行中的程序(先尝试优雅退出;失败强制结束)
Function .onInit
  nsExec::ExecToStack 'tasklist /FI "IMAGENAME eq EOL_CAN_Tool.exe" /NH'
  Pop $0
  Pop $1
  ${If} $1 != ""
    nsExec::Exec 'taskkill /IM EOL_CAN_Tool.exe /F'
    Sleep 500
  ${EndIf}
FunctionEnd

Section "主程序" SEC_MAIN
  SetOutPath "$INSTDIR"
  ; 升级时先清理旧版 Qt 插件目录(.o/.obj 等编译产物与旧 DLL 一律不带入)
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\styles"
  RMDir /r "$INSTDIR\tls"
  RMDir /r "$INSTDIR\iconengines"
  RMDir /r "$INSTDIR\imageformats"
  RMDir /r "$INSTDIR\networkinformation"
  RMDir /r "$INSTDIR\generic"
  RMDir /r "$INSTDIR\canbus"
  RMDir /r "$INSTDIR\multimedia"
  ; 升级保留用户配置文件:eol_tool_cfg.ini 等由程序运行后在安装目录生成,
  ; 不在包内,故 File /r 不会覆盖/删除它们
  File /r /x "*.o" /x "*.obj" /x "*.res" /x "*.cpp" /x "*.h" /x "object_script*" /x "Makefile*" /x ".qmake.stash" "${SRC}\*.*"
  WriteRegStr HKCU "Software\EOL_CAN_Tool" "InstallDir" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\EOL_CAN_Tool" \
    "DisplayName" "EOL CAN Tool"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\EOL_CAN_Tool" \
    "DisplayIcon" "$INSTDIR\EOL_CAN_Tool.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\EOL_CAN_Tool" \
    "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\EOL_CAN_Tool" \
    "Publisher" "aron566"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\EOL_CAN_Tool" \
    "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "$INSTDIR\uninstall.exe"

  CreateDirectory "$SMPROGRAMS\EOL CAN Tool"
  CreateShortcut "$SMPROGRAMS\EOL CAN Tool\EOL CAN Tool.lnk" \
    "$INSTDIR\EOL_CAN_Tool.exe" "" "$INSTDIR\EOL_CAN_Tool.exe" 0
  CreateShortcut "$SMPROGRAMS\EOL CAN Tool\卸载.lnk" \
    "$INSTDIR\uninstall.exe"
  CreateShortcut "$DESKTOP\EOL CAN Tool.lnk" \
    "$INSTDIR\EOL_CAN_Tool.exe" "" "$INSTDIR\EOL_CAN_Tool.exe" 0
SectionEnd

Section "Uninstall"
  nsExec::Exec 'taskkill /IM EOL_CAN_Tool.exe /F'
  Sleep 300
  Delete "$DESKTOP\EOL CAN Tool.lnk"
  RMDir /r "$SMPROGRAMS\EOL CAN Tool"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\EOL_CAN_Tool"
  DeleteRegKey HKCU "Software\EOL_CAN_Tool"
SectionEnd
