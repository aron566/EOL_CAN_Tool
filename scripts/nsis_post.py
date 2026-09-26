# -*- coding: utf-8 -*-
"""NSIS 打包后处理:补 VC 运行库 + 第三方 CAN 驱动 DLL + 数据目录.

用法: nsis_post.py <nsis_pkg目录>
windeployqt 只收集 Qt 依赖(不含 MSVC 运行库与第三方 CAN 驱动库),这里补齐。
所有源路径均为 x64 且已纳入 git 跟踪,保证 CI 全新克隆也能复现。
"""
import os
import glob
import shutil
import sys

PKG = sys.argv[1]
PROJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# 顶层第三方 DLL(源路径相对工程根,均已纳入 git 跟踪)
VENDOR_DLLS = {
    'zlgcan.dll': 'zlg_can_lib/zlgcan_x64/zlgcan.dll',
    'ECanVci64.dll': 'gc_can_lib/GCANx64/ECanVci64.dll',
    'ECANFDVCI64.dll': 'gc_can_lib/GCANx64/ECANFDVCI64.dll',
    'GCANUSB_x64.dll': 'gc_can_lib/GCANx64/GCANUSB_x64.dll',
    'CHUSBDLL64.dll': 'gc_can_lib/GCANx64/CHUSBDLL64.dll',
    'hv.dll': '3third_party_lib/hv/bin/hv.dll',
    'libcrypto-1_1-x64.dll': '3third_party_lib/hv/lib/libcrypto-1_1-x64.dll',
    'libssl-1_1-x64.dll': '3third_party_lib/hv/lib/libssl-1_1-x64.dll',
    'binlog.dll': '3third_party_lib/blf/LIB/x64_Release/binlog.dll',
}

# 整目录复制(含驱动所需的 ini/xml 配置文件)
DATA_DIRS = {
    'kerneldlls': 'zlg_can_lib/zlgcan_x64/kerneldlls',
    'Configuration': 'ts_can_lib/ts_can_x64/Configuration',
}

# 仅复制目录内 DLL(排除 .lib/.def/.exp 编译产物)
DLL_ONLY_DIRS = {
    'kvaser_can_x64': 'kvaser_can_lib/kvaser_can_x64',
    'ts_can_x64': 'ts_can_lib/ts_can_x64',
}


def find_vc_crt():
    """定位 VC143 运行库目录(兼容 Community/Enterprise/Professional/BuildTools)。"""
    bases = [
        r'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC',
        r'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Redist\MSVC',
        r'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Redist\MSVC',
        r'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Redist\MSVC',
    ]
    for base in bases:
        if not os.path.isdir(base):
            continue
        for v in sorted(os.listdir(base), reverse=True):
            crt = os.path.join(base, v, 'x64', 'Microsoft.VC143.CRT')
            if os.path.isdir(crt):
                return crt
    raise SystemExit('找不到 VC143.CRT 目录')


def main():
    # 0. 删除冗余的 vc_redist 安装器(下面已补 VC 运行库 DLL,无需再带安装器)
    removed = 0
    for f in glob.glob(os.path.join(PKG, 'vc_redist*.exe')):
        os.remove(f)
        removed += 1
    if removed:
        print(f'[0/4] 删除冗余 vc_redist 安装器: {removed} 个')

    # 1. 补 MSVC 运行库
    vc = find_vc_crt()
    copied = 0
    for f in os.listdir(vc):
        if f.endswith('.dll'):
            shutil.copy2(os.path.join(vc, f), os.path.join(PKG, f))
            copied += 1
    print(f'[1/4] 补 VC 运行库: {copied} 个')

    # 2. 顶层第三方 DLL
    for name, rel in VENDOR_DLLS.items():
        src = os.path.join(PROJ, rel)
        if not os.path.isfile(src):
            raise SystemExit(f'缺少 {src},请确认第三方库已提交到 git')
        shutil.copy2(src, os.path.join(PKG, name))
    print(f'[2/4] 补第三方 DLL: {len(VENDOR_DLLS)} 个')

    # 3. 数据目录(整目录)
    for name, rel in DATA_DIRS.items():
        src = os.path.join(PROJ, rel)
        dst = os.path.join(PKG, name)
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(src, dst)
    print(f'[3/4] 补数据目录: {list(DATA_DIRS)}')

    # 4. 仅 DLL 目录
    for name, rel in DLL_ONLY_DIRS.items():
        src = os.path.join(PROJ, rel)
        dst = os.path.join(PKG, name)
        os.makedirs(dst, exist_ok=True)
        n = 0
        for f in os.listdir(src):
            if f.lower().endswith('.dll'):
                shutil.copy2(os.path.join(src, f), os.path.join(dst, f))
                n += 1
        print(f'[4/4] 补 {name}: {n} 个 DLL')
    print('后处理完成')


if __name__ == '__main__':
    main()
