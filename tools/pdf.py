"""report/REPORT.md를 제출용 PDF(report/REPORT.pdf)로 만든다.

    python3 tools/pdf.py

마크다운 → HTML은 Python `markdown` 모듈, HTML → PDF는 Chromium(Playwright)의
인쇄 기능을 쓴다. 둘 다 이 저장소의 컨테이너에는 없으므로 과제 제출용 PDF를 만들
때만 쓰는 보조 스크립트다. (없으면 VS Code의 Markdown PDF 확장이나 브라우저의
"PDF로 저장"으로 같은 일을 할 수 있다.)
"""

from pathlib import Path

import markdown
from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "report" / "REPORT.md"
HTML = ROOT / "report" / "REPORT.html"
PDF = ROOT / "report" / "REPORT.pdf"

CSS = """
@page { size: A4; margin: 15mm 15mm 16mm 15mm; }
body { font-family: 'Noto Sans CJK KR', 'Noto Sans KR', sans-serif; font-size: 9.6pt;
       line-height: 1.55; color: #1f2328; }
h1 { font-size: 17pt; margin: 0 0 8px; border-bottom: 2px solid #1f2328; padding-bottom: 4px; }
h2 { font-size: 13pt; margin: 16px 0 6px; border-bottom: 1px solid #d0d7de; padding-bottom: 2px;
     break-after: avoid; }
h3 { font-size: 11pt; margin: 12px 0 4px; break-after: avoid; }
p { margin: 4px 0 6px; }
ul, ol { margin: 4px 0 6px; padding-left: 20px; }
li { margin: 2px 0; }
table { border-collapse: collapse; margin: 6px 0 8px; font-size: 8.6pt; break-inside: avoid; }
th, td { border: 1px solid #d0d7de; padding: 2px 6px; }
th { background: #f3f4f6; }
code { font-family: 'DejaVu Sans Mono', monospace; font-size: 8.4pt; background: #f3f4f6;
       padding: 0 3px; border-radius: 3px; }
pre { background: #f6f8fa; border: 1px solid #e5e7eb; padding: 6px 8px; font-size: 8.2pt;
      line-height: 1.4; break-inside: avoid; margin: 6px 0; }
pre code { background: none; padding: 0; }
blockquote { border-left: 3px solid #0072B2; margin: 6px 0; padding: 2px 10px; color: #333;
             background: #f5f9fc; }
img { display: block; width: 82%; margin: 6px auto; break-inside: avoid; }
hr { border: none; border-top: 1px solid #d0d7de; margin: 10px 0; }
"""


def main():
    body = markdown.markdown(SRC.read_text(encoding="utf-8"),
                             extensions=["tables", "fenced_code"])
    HTML.write_text(f"<!doctype html><html lang='ko'><head><meta charset='utf-8'>"
                    f"<title>정렬 비교 보고서</title><style>{CSS}</style></head>"
                    f"<body>{body}</body></html>", encoding="utf-8")
    with sync_playwright() as pw:
        browser = pw.chromium.launch()
        page = browser.new_page()
        page.goto(HTML.as_uri())
        page.wait_for_timeout(500)
        page.pdf(path=str(PDF), format="A4", print_background=True,
                 display_header_footer=True, header_template="<span></span>",
                 footer_template="<div style='font-size:8px;width:100%;text-align:center;"
                                 "color:#888'><span class='pageNumber'></span> / "
                                 "<span class='totalPages'></span></div>",
                 margin={"top": "15mm", "bottom": "16mm", "left": "15mm", "right": "15mm"})
        browser.close()
    HTML.unlink()
    print(f"wrote {PDF.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
