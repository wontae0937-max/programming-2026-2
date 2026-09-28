"""보고서 2.3의 힙 그림(report/figures/heap-tree.svg)을 그린다. 표준 모듈만 쓴다.

강의 예제 배열 2 8 5 9 1 10 7 6 4 3 에 1단계(힙 만들기)를 돌린 결과를
트리와 배열로 나란히 놓아, 같은 것을 두 가지로 본다는 점을 보여 준다.
"""

from pathlib import Path

import svgchart as sc

HEAP = [10, 9, 7, 8, 3, 5, 2, 6, 4, 1]
OUT = Path(__file__).resolve().parents[1] / "report" / "figures" / "heap-tree.svg"
BLUE, TEAL = "#0072B2", "#009E73"


def main():
    c = sc.Canvas(720, 330, "최대 힙은 트리이자 배열이다",
                  "강의 예제 배열 2 8 5 9 1 10 7 6 4 3 → 1단계(힙 만들기)를 마친 모습")
    # 트리: 깊이 d의 k번째 노드를 가로로 고르게 편다.
    pos = {}
    for i, _ in enumerate(HEAP):
        depth = (i + 1).bit_length() - 1
        k = i + 1 - (1 << depth)
        width = 440 / (1 << depth)
        pos[i] = (20 + width * (k + 0.5), 102 + depth * 52)
    for i in range(len(HEAP)):
        for child in (2 * i + 1, 2 * i + 2):
            if child < len(HEAP):
                (x1, y1), (x2, y2) = pos[i], pos[child]
                c.line(x1, y1, x2, y2, stroke=sc.MUTED, width=1.4)
    for i, v in enumerate(HEAP):
        x, y = pos[i]
        fill = BLUE if i == 0 else "#ffffff"
        c.parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="17" fill="{fill}" '
                       f'stroke="{BLUE}" stroke-width="1.6"/>')
        c.text(x, y + 5, v, size=14, anchor="middle", weight="600",
               fill="#ffffff" if i == 0 else sc.INK)
        c.text(x + 21, y - 12, f"[{i}]", size=10, fill=sc.MUTED)
    c.text(pos[0][0] - 26, pos[0][1] + 5, "최댓값은 늘 뿌리", size=11, fill=BLUE, weight="600", anchor="end")

    # 배열: 같은 원소를 인덱스 순서로.
    x0, y0, w = 470, 120, 24
    c.text(x0, y0 - 16, "배열 a[0..9]", size=12, weight="600")
    for i, v in enumerate(HEAP):
        x = x0 + i * w
        c.parts.append(f'<rect x="{x}" y="{y0}" width="{w - 2}" height="28" fill="#ffffff" '
                       f'stroke="{BLUE if i == 0 else sc.MUTED}" stroke-width="1.3"/>')
        c.text(x + (w - 2) / 2, y0 + 19, v, size=12, anchor="middle")
        c.text(x + (w - 2) / 2, y0 + 44, i, size=10, anchor="middle", fill=sc.MUTED)
    lines = [
        "부모 i 의 자식 → 2i+1, 2i+2",
        "자식 i 의 부모 → (i−1)/2",
        "예) a[1]=9 의 자식은 a[3]=8, a[4]=3",
        "규칙은 하나: 부모 ≥ 자식",
        "형제끼리의 순서는 정하지 않는다",
    ]
    for k, s in enumerate(lines):
        c.text(x0, y0 + 84 + k * 20, s, size=12, fill=TEAL if k == 3 else sc.INK,
               weight="600" if k == 3 else "400")
    c.save(OUT)
    print(f"wrote {OUT.relative_to(OUT.parents[2])}")


if __name__ == "__main__":
    main()
