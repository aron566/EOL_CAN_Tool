# -*- coding: utf-8 -*-
"""生成中英文使用说明 PDF 到打包目录 docs/。

流程:docs/*.md -> html(pandoc 优先,回退 python-markdown) -> Edge/Chrome headless 打印 PDF。
用法: make_docs.py <输出目录>   (如 make_docs.py nsis_pkg/docs)
"""
import os
import shutil
import subprocess
import sys

# CI 的 Python stdout 可能是 cp1252,中文 print 会 UnicodeEncodeError;强制 UTF-8
try:
    sys.stdout.reconfigure(encoding='utf-8')
    sys.stderr.reconfigure(encoding='utf-8')
except Exception:
    pass

OUT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else 'nsis_pkg/docs')
PROJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(PROJ, 'docs')

MANUALS = [
    ('USER_MANUAL_zh-CN.md', 'EOL_CAN_Tool使用说明书.pdf', 'EOL CAN Tool 使用说明书'),
    ('USER_MANUAL_en.md', 'EOL_CAN_Tool_User_Manual.pdf', 'EOL CAN Tool User Manual'),
]

_CSS = ("body{font-family:'Microsoft YaHei',sans-serif;max-width:900px;"
        "margin:24px auto;padding:0 16px;line-height:1.6;}"
        "table{border-collapse:collapse;width:100%;}"
        "th,td{border:1px solid #bbb;padding:6px 10px;text-align:left;}"
        "th{background:#f0f0f0;} code{background:#f5f5f5;padding:1px 4px;border-radius:3px;}"
        "pre{background:#f5f5f5;padding:10px;border-radius:4px;overflow-x:auto;}")


def find_browser():
    for p in (r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe',
              r'C:\Program Files\Google\Chrome\Application\chrome.exe',
              r'C:\Program Files (x86)\Google\Chrome\Application\chrome.exe'):
        if os.path.isfile(p):
            return p
    raise SystemExit('找不到 Edge/Chrome,无法生成 PDF')


def md_to_html(md_path, title):
    html_path = md_path + '.html'
    if shutil.which('pandoc'):
        subprocess.run(['pandoc', md_path, '-f', 'gfm', '-t', 'html5', '-s',
                        '--metadata', 'title=' + title, '-o', html_path], check=True)
    else:
        import markdown
        md = open(md_path, encoding='utf-8').read()
        body = markdown.markdown(md, extensions=['tables', 'fenced_code'])
        html = ('<html><head><meta charset="utf-8"><title>' + title +
                '</title><style>' + _CSS + '</style></head><body>' + body + '</body></html>')
        with open(html_path, 'w', encoding='utf-8') as f:
            f.write(html)
    return html_path


def main():
    os.makedirs(OUT, exist_ok=True)
    browser = find_browser()
    for md_name, pdf_name, title in MANUALS:
        md_path = os.path.join(DOCS, md_name)
        if not os.path.isfile(md_path):
            raise SystemExit('缺少 ' + md_path)
        html_path = md_to_html(md_path, title)
        out_pdf = os.path.join(OUT, pdf_name)
        html_url = 'file:///' + html_path.replace('\\', '/')
        subprocess.run([browser, '--headless', '--disable-gpu', '--no-pdf-header-footer',
                        '--print-to-pdf=' + out_pdf, html_url], check=True)
        os.remove(html_path)
        print('生成: ' + out_pdf)


if __name__ == '__main__':
    main()
