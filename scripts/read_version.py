# -*- coding: utf-8 -*-
"""从 resource/EOL_CAN_Tool.rc 读取产品版本号(VER_PRODUCTVERSION_STR)。

版本号唯一事实来源是 .rc;打包脚本与 CI 一致性校验都从这里读,
保证「改一处、处处同步」。
"""
import os
import re
import sys

PROJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RC = os.path.join(PROJ, 'resource', 'EOL_CAN_Tool.rc')


def read_version():
    with open(RC, 'r', encoding='utf-8', errors='replace') as f:
        text = f.read()
    m = re.search(r'VER_PRODUCTVERSION_STR\s+"([0-9.]+)', text)
    if not m:
        raise SystemExit('未在 resource/EOL_CAN_Tool.rc 中找到 VER_PRODUCTVERSION_STR')
    return m.group(1)


if __name__ == '__main__':
    v = read_version()
    if '--out' in sys.argv:
        # 写入文件(不带换行),供 cmd 的 set /p 读取(避开 for /f 反引号引号陷阱)
        with open(sys.argv[sys.argv.index('--out') + 1], 'w', encoding='ascii') as f:
            f.write(v)
    else:
        print(v)
