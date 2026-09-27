#!/usr/bin/env bash
# Linux CI 打包: linuxdeploy + linuxdeploy-plugin-qt 生成 AppImage 单文件安装包。
# 厂商驱动 .so(周立功 x3 + 同星 x2)与主程序同目录,RUNPATH=$ORIGIN,故打包时
# 把它们复制到 AppDir/usr/bin(主程序所在目录)而不是 usr/lib。
# 用法: bash scripts/package_linux.sh <版本号> <构建产物目录>
#   <构建产物目录>: 含 EOL_CAN_Tool 可执行文件与厂商 .so 的目录(qmake DESTDIR)。
set -e
cd "$(dirname "$0")/.."
PROJ="$PWD"
VER="${1:?用法: package_linux.sh <版本号> <产物目录>}"
BINDIR="${2:?用法: package_linux.sh <版本号> <产物目录>}"
BIN="$BINDIR/EOL_CAN_Tool"
[ -x "$BIN" ] || { echo "错误: 找不到可执行文件 $BIN"; exit 1; }

WORK=/tmp/appimage-pkg
rm -rf "$WORK"; mkdir -p "$WORK"; cd "$WORK"

echo "== 1/4 下载 linuxdeploy 及 Qt 插件"
curl -sSL -o linuxdeploy \
  https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
curl -sSL -o linuxdeploy-plugin-qt \
  https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy linuxdeploy-plugin-qt
# runner 无 FUSE,直接解压 AppImage 运行
./linuxdeploy --appimage-extract >/dev/null && mv squashfs-root linuxdeploy-root
./linuxdeploy-plugin-qt --appimage-extract >/dev/null && mv squashfs-root linuxdeploy-plugin-qt-root
printf '#!/bin/sh\nexec "%s/linuxdeploy-plugin-qt-root/AppRun" "$@"\n' "$WORK" \
  > "$WORK/linuxdeploy-plugin-qt"
chmod +x "$WORK/linuxdeploy-plugin-qt"
export PATH="$WORK:$PATH"
LD="$WORK/linuxdeploy-root/AppRun"
# 系统 Qt(qmake6)或 install-qt-action 的 Qt(qmake)均可
if command -v qmake >/dev/null; then :;
elif command -v qmake6 >/dev/null; then
  printf '#!/bin/sh\nexec qmake6 "$@"\n' > "$WORK/qmake"; chmod +x "$WORK/qmake"
else
  echo "错误: PATH 中找不到 qmake/Qt 6"; exit 1
fi

echo "== 2/4 部署 Qt 依赖到 AppDir"
# 图标已预置为 256x256 标准尺寸(linuxdeploy 拒收 44x44 等非标准尺寸)
cp "$PROJ/scripts/EOL_CAN_Tool.png" ./EOL_CAN_Tool.png
"$LD" --appdir AppDir \
  --executable "$BIN" \
  --desktop-file "$PROJ/scripts/EOL_CAN_Tool.desktop" \
  --icon-file ./EOL_CAN_Tool.png \
  --plugin qt

echo "== 3/4 放入厂商驱动 .so(与主程序同目录)"
n=0
# 注意: libusbcanfd800u.so 中间没有连字符,glob 不能写成 libusbcan-*.so
for so in "$BINDIR"/libusbcan*.so "$BINDIR"/libTSCANApiOnLinux.so "$BINDIR"/libTSH.so; do
  if [ -f "$so" ]; then
    cp -v "$so" AppDir/usr/bin/
    n=$((n + 1))
  fi
done
if [ "$n" -eq 0 ]; then
  echo "错误: $BINDIR 下未找到厂商驱动 .so"; exit 1
elif [ "$n" -lt 5 ]; then
  echo "警告: 仅找到 $n 个厂商驱动 .so(期望 5 个:周立功 x3 + 同星 x2)"
else
  echo "已放入 $n 个厂商驱动 .so"
fi

echo "== 3b/4 补齐 offscreen/minimal 平台插件(插件只自动部署 xcb)"
QT_PLUGINS=$(qmake -query QT_INSTALL_PLUGINS)
for p in libqoffscreen.so libqminimal.so; do
  src="$QT_PLUGINS/platforms/$p"
  if [ -f "$src" ]; then
    cp -v "$src" AppDir/usr/plugins/platforms/
  else
    echo "警告: 未找到 $src"
  fi
done

echo "== 4/4 生成 AppImage"
export VERSION="$VER"
"$LD" --appdir AppDir --output appimage
mkdir -p "$PROJ/dist"
mv -v EOL_CAN_Tool-*.AppImage "$PROJ/dist/"
ls -la "$PROJ/dist/"
echo "DONE"
