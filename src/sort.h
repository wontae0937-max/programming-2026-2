/* 정렬 비교 과제 — 병합 · 퀵 · 힙 정렬을 하나의 공통 인터페이스로 묶는다.
 *
 * C에는 interface·class가 없다. 대신 **함수 포인터를 담은 구조체**를 쓴다.
 * 자바의 interface 구현체 하나가 여기서는 SortAlgorithm 값 하나에 해당하고,
 * 비교 규약은 표준 라이브러리의 qsort와 똑같이 맞췄다.
 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>

/* 비교 함수. qsort와 같은 규약: a<b면 음수, a==b면 0, a>b면 양수.
 * 이 함수 포인터 덕분에 정렬이 원소의 타입을 몰라도 된다. */
typedef int (*SortCompare)(const void *a, const void *b);

/* 한 번 정렬하는 동안 모인 측정값. 시간은 바깥(bench)에서 재고,
 * 여기에는 시계와 무관하게 재현되는 값만 담는다. */
typedef struct SortStats {
    size_t compares;   /* 비교 함수를 부른 횟수 */
    size_t moves;      /* 원소를 복사한 횟수 (교환 한 번은 3) */
    size_t extraBytes; /* 입력 배열 밖에 잡은 작업 공간의 최대 바이트 */
    size_t maxDepth;   /* 재귀 깊이의 최댓값. 반복문만 쓰면 1 */
} SortStats;

/* 정렬 한 가지. 이 구조체가 이 과제의 "인터페이스"다. */
typedef struct SortAlgorithm {
    const char *name;
    const char *timeComplexity;  /* 평균 시간복잡도 (표에 찍는 설명) */
    const char *spaceComplexity; /* 추가 메모리 */
    int stable;                  /* 안정 정렬이라고 주장하는 값. 테스트가 실측과 맞춰 본다 */
    /* base[0..n-1]을 제자리에서 오름차순 정렬한다. 원소 하나는 size 바이트다.
     * stats가 NULL이면 측정하지 않는다. */
    void (*sort)(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
} SortAlgorithm;

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

/* 강의(주제 04) 그대로 첫 원소를 피벗으로 쓰는 퀵 정렬. 비교 대상 셋에는
 * 넣지 않고, "랜덤 피벗이 무엇을 막는가"를 재는 실험에서만 쓴다. */
void quickSortFirstPivot(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

/* 비교 대상인 세 정렬. 호출하는 쪽은 이 표만 훑으면 된다.
 * 정렬을 하나 더 만들면 표에 한 줄 넣는 것으로 끝난다. */
extern const SortAlgorithm SORT_ALGORITHMS[];
extern const size_t SORT_ALGORITHM_COUNT;

/* 실험용 변형. 테스트는 이 표도 훑는다 — 실험에만 쓰더라도 정렬은 맞아야 한다. */
extern const SortAlgorithm SORT_VARIANTS[];
extern const size_t SORT_VARIANT_COUNT;

void sortStatsReset(SortStats *stats);
int sortCompareInt(const void *a, const void *b); /* int 배열용 기본 비교 함수 */

#endif /* SORT_H */
