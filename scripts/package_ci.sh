#!/usr/bin/env bash
# CI 专用打包脚本(本地打包请用 package_nsis.bat):
# qmake+nmake 构建在 workflow 的 vcvars 步骤完成,这里只做 windeployqt + 后处理 + NSIS。
# 不硬编码本机 Qt/NSIS 路径 —— Qt 由 install-qt-action 装入 PATH,NSIS 由 choco 安装。
# 用法: bash scripts/package_ci.sh <版本号>
set -e
cd "$(dirname "$0")/.."
VER="${1:?用法: package_ci.sh <版本号>}"

echo "== 1/4 windeployqt 收集运行时"
rm -rf nsis_pkg
mkdir -p nsis_pkg
cp bin/EOL_CAN_Tool.exe nsis_pkg/
windeployqt --release --no-translations nsis_pkg/EOL_CAN_Tool.exe

echo "== 2/4 后处理 (VC 运行库 + CAN 驱动 DLL + 数据目录)"
python scripts/nsis_post.py nsis_pkg

echo "== 3/4 生成使用说明 PDF"
python scripts/make_docs.py nsis_pkg/docs

echo "== 4/4 NSIS 打包"
mkdir -p dist
# makensis:choco 装到 'C:\Program Files (x86)\NSIS' 但未必进当前 shell PATH,显式补上
if ! command -v makensis >/dev/null 2>&1; then
    if [ -x "/c/Program Files (x86)/NSIS/makensis.exe" ]; then
        export PATH="/c/Program Files (x86)/NSIS:$PATH"
    elif [ -x "/c/Program Files/NSIS/makensis.exe" ]; then
        export PATH="/c/Program Files/NSIS:$PATH"
    fi
fi
SRCWIN=$(cygpath -w "$PWD/nsis_pkg")
# installer.nsi 内的 OutFile/图标等相对路径是相对 CWD 解析的,故在 scripts/ 下运行 makensis
cd scripts
makensis -DVERSION="$VER" "-DSRC=$SRCWIN" installer.nsi
cd ..
ls -la "dist/EOL_CAN_Tool_Setup_v${VER}.exe"
echo "DONE: dist/EOL_CAN_Tool_Setup_v${VER}.exe"
