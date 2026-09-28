# Changelog

이 저장소(2026-2 고급알고리즘 과제 1 — 정렬 비교)의 변경 기록. 형식은
[Keep a Changelog](https://keepachangelog.com/ko/1.1.0/)를 따른다.

## [1.0] - 2026-09-30

병합 · 퀵 · 힙 정렬을 비교한 첫 제출본.

### Added

- **정렬 셋**: `mergeSort`(주제 03 강의 코드와 같은 모양), `quickSort`(주제 04의
  파티션 + minstd 난수로 고른 랜덤 피벗), `heapSort`(수업에서 다루지 않은 정렬,
  구멍 내리기 `siftDown`). 실험용 변형 `quickSortFirstPivot`은 `SORT_VARIANTS`에 따로 둔다.
- **강의 숫자 재현 테스트**: 주제 03 · 04 슬라이드의 비교 · 이동 횟수 7개를 그대로
  내는지 본다. 테스트는 모두 67개.
- **실험 3 · 4** (`--pivot`, `--dups`): 첫 원소 피벗 vs 랜덤 피벗, 서로 다른 key의
  개수를 바꿔 가며 잰 비교.
- `tools/heapfig.py`(힙 그림), `tools/pdf.py`(제출용 PDF).

### Changed (샘플 저장소 대비)

- 시간은 평균이 아니라 **reps회 중 최솟값**을 남긴다.
- 같은 key가 하나도 없으면 안정성을 **판정 불가(-1, 표에서는 `-`)**로 둔다.
- `sortSwap`은 자기 자신과의 교환을 세지 않는다(강의의 이동 횟수와 맞추기 위해).
