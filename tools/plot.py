"""비교 결과를 그래프(SVG)로 그린다.

    make charts          # 또는
    python3 tools/plot.py

src/main.out 을 --csv · --pivot · --dups 로 돌려 측정값을 받고, report/ 아래에
CSV 원본과 SVG를 쓴다. 사람이 읽는 표를 파싱하지 않고 CSV를 쓰는 이유는, 표의
모양이 바뀌어도 그래프가 깨지지 않게 하려는 것이다.

표준 모듈만 쓴다. 그림은 tools/svgchart.py가 직접 찍어 낸다.
"""

import csv
import math
import io
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import svgchart  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "src" / "main.out"
OUT_DIR = ROOT / "report"
FIG_DIR = OUT_DIR / "figures"
ALGOS = ["mergeSort", "quickSort", "heapSort"]
KIND_KEYS = ["random", "sorted", "reversed", "few-unique"]
KIND_LABEL = {
    "random": "무작위",
    "sorted": "정렬됨",
    "reversed": "역순",
    "few-unique": "중복많음",
}
INT_FIELDS = ("n", "compares", "moves", "extraBytes", "maxDepth", "distinct")


def run_csv(flag, save_as):
    """측정 프로그램을 돌려 CSV를 읽고, 원본을 report/에 그대로 남긴다."""
    if not BINARY.exists():
        subprocess.run(["make", "src/main.out"], cwd=ROOT, check=True)
    result = subprocess.run([str(BINARY), flag], cwd=ROOT, check=True,
                            capture_output=True, text=True)
    (OUT_DIR / save_as).write_text(result.stdout, encoding="utf-8")
    rows = list(csv.DictReader(io.StringIO(result.stdout)))
    for row in rows:
        for key in INT_FIELDS:
            if key in row:
                row[key] = int(row[key])
        row["millis"] = float(row["millis"])
    return rows


def pick(rows, **conditions):
    return [r for r in rows if all(r[k] == v for k, v in conditions.items())]


def by_algo(rows, field, keys, key_field, algos=ALGOS):
    """{알고리즘: [키 순서대로의 값]} 으로 모은다."""
    table = {}
    for algo in algos:
        values = []
        for key in keys:
            match = [r for r in rows if r["algo"] == algo and r[key_field] == key]
            values.append(match[0][field] if match else 0)
        table[algo] = values
    return table


def main():
    OUT_DIR.mkdir(exist_ok=True)
    FIG_DIR.mkdir(exist_ok=True)
    rows = run_csv("--csv", "results.csv")
    pivot = run_csv("--pivot", "pivot.csv")
    dups = run_csv("--dups", "dups.csv")
    made = []

    kinds = pick(rows, scope="kinds")
    growth = pick(rows, scope="growth")
    n_kinds = kinds[0]["n"]
    sizes = sorted({r["n"] for r in growth})
    labels = [KIND_LABEL[k] for k in KIND_KEYS]

    # 실험 1 · 입력 모양별. 중복많음의 퀵이 다른 값보다 100배 커서 로그 축으로 그린다.
    made.append(svgchart.grouped_bar_chart(
        FIG_DIR / "1-shapes-compares.svg",
        "입력 모양에 따른 비교 횟수 — 로그 축",
        f"n = {n_kinds:,} · 눈금 한 칸이 10배다. 중복많음에서 퀵 정렬만 무작위 입력의 78배로 뛴다",
        labels, by_algo(kinds, "compares", KIND_KEYS, "input"), "비교 횟수",
        log_scale=True))
    made.append(svgchart.grouped_bar_chart(
        FIG_DIR / "1-shapes-time.svg",
        "입력 모양에 따른 걸린 시간 — 로그 축",
        f"n = {n_kinds:,} · 5회 중 가장 빠른 회차",
        labels, by_algo(kinds, "millis", KIND_KEYS, "input"), "시간 (ms)",
        log_scale=True, value_label=svgchart.ms))

    # 실험 2 · n을 키우며
    made.append(svgchart.line_chart(
        FIG_DIR / "2-growth-compares.svg",
        "n이 커질 때 비교 횟수 — 로그-로그 축",
        "무작위 입력 · 기울기가 곧 복잡도 지수다 (n log n이면 1보다 조금 크다)",
        sizes, by_algo(growth, "compares", sizes, "n"), "n (원소 개수)", "비교 횟수"))
    # 시간을 n log2 n으로 나누면, 순수한 n log n이라면 평평한 선이 된다.
    # 선이 오르는 만큼이 복잡도 바깥의 비용(캐시 미스 등)이다.
    per = {}
    for algo, values in by_algo(growth, "millis", sizes, "n").items():
        per[algo] = [ms * 1e6 / (n * math.log2(n)) for ms, n in zip(values, sizes)]
    made.append(svgchart.line_chart(
        FIG_DIR / "2-growth-time.svg",
        "n log n 한 단위당 걸린 시간 — 평평하면 순수한 n log n",
        "무작위 입력 · 시간(ns) ÷ (n·log₂n) · 오르는 만큼이 복잡도 밖의 비용이다",
        sizes, per, "n (원소 개수, 로그 축)", "ns / (n·log₂n)",
        log_axes=False, log_x=True))
    biggest = pick(growth, n=sizes[-1])
    made.append(svgchart.grouped_bar_chart(
        FIG_DIR / "2-compares-vs-moves.svg",
        f"비교와 이동 — 서로 다른 것을 아낀다 (n = {sizes[-1]:,})",
        "병합은 비교를, 힙은 이동을 가장 적게 쓴다. 퀵은 양쪽 다 중간이다",
        ["비교", "이동"],
        {a: [pick(biggest, algo=a)[0]["compares"], pick(biggest, algo=a)[0]["moves"]]
         for a in ALGOS},
        "횟수"))

    # 실험 3 · 피벗
    pair = ["quickFirst", "quickSort"]
    made.append(svgchart.grouped_bar_chart(
        FIG_DIR / "3-pivot-compares.svg",
        "첫 원소 피벗 vs 랜덤 피벗 — 비교 횟수 (로그 축)",
        f"n = {pivot[0]['n']:,} · 정렬됨·역순에서 첫 원소 피벗은 n(n-1)/2로 무너진다",
        labels, by_algo(pivot, "compares", KIND_KEYS, "input", pair), "비교 횟수",
        log_scale=True))

    # 실험 4 · 중복
    distincts = sorted({r["distinct"] for r in dups})
    made.append(svgchart.line_chart(
        FIG_DIR / "4-dups-compares.svg",
        "서로 다른 key가 줄어들 때 비교 횟수 — 로그-로그 축",
        f"n = {dups[0]['n']:,} · 왼쪽으로 갈수록 중복이 많다. 퀵만 거꾸로 폭발한다",
        distincts, by_algo(dups, "compares", distincts, "distinct"),
        "서로 다른 key의 개수", "비교 횟수", annotate_slope=False))

    for path in made:
        print(f"wrote {Path(path).relative_to(ROOT)}")


if __name__ == "__main__":
    main()
