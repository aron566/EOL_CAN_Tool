@echo off
REM 设置bin文件路径
set bin_path=C:/Users/EDY/Desktop/EOL_CAN_Tool
set bin_file=EOL_CAN_Tool.exe
REM 设置额外拷贝目录
set extra_data=%bin_path%/kvaser_can_x64,%bin_path%/kerneldlls,%bin_path%/Configuration,%bin_path%/ts_can_x64
set qmake_dir=D:/QT_Install/5.15.2/mingw81_64/bin/qmake.exe
REM 设置额外库文件路径
set extralib_dir=C:/Users/EDY/Desktop/EOL_CAN_Tool
REM 设置额外库文件
set extra_libs=zlgcan ECanVci64 ECANFDVCI64 GCANUSB_x64 libcrypto-1_1-x64 libhv libssl-1_1-x64 binlog
REM 设置作者
set author=aron566
REM 设置版本
set version=v1.4.3
REM 设置文件图标
set icon_file=D:/QT_Workspace/EOL_CAN_Tool/resource/icons/exe_icon.ico
REM 安装文件图标
set log_file=D:/QT_Workspace/EOL_CAN_Tool/resource/icons/icon.ico
set banner_file=D:/QT_Workspace/EOL_CAN_Tool/resource/icons/icon.ico
set cmd=CQtDeployer.exe -bin %bin_file% -binPrefix %bin_path% -icon %icon_file% -qifLogo %log_file% -qifBanner %banner_file% -deployVersion %version% -publisher %author% -releaseDate %DATE% -extraData %extra_data% -qmake %qmake_dir% -libDir %extralib_dir% -extraLibs %extra_libs% qif
@echo.
echo %cmd%
pause

REM 执行命令
%cmd%
