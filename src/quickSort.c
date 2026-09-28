/* 퀵 정렬 — 피벗을 제자리에 놓고, 양쪽을 각각 정렬한다 (주제 04).
 *
 * 병합 정렬과 시소의 반대편이다. 일은 들어가는 길의 파티션에서 하고, 돌아온
 * 뒤에는 할 일이 없다. temp 배열이 필요 없고 이동이 적다. 대신 피벗이 한쪽
 * 끝으로 쏠리면 O(n^2)이 되고, 멀리 떨어진 원소를 맞바꾸므로 불안정하다.
 *
 * 파티션은 강의 코드(02_quick_sort/quickSort.c)와 같다. 피벗을 a[lo]에 두고
 * 그보다 작은 것을 왼쪽으로 모은 뒤, 피벗을 그 경계로 옮긴다.
 *
 * 두 가지를 둔다.
 *   quickSort            — 파티션마다 피벗을 무작위로 고른다. 비교 대상은 이쪽이다.
 *   quickSortFirstPivot  — 강의 그대로 첫 원소가 피벗이다. 피벗 실험에만 쓴다.
 */
#include "sort.h"

#include <stdint.h>

#include "sortctx.h"

/* --- 난수 (주제 04, 01_random) ---------------------------------------- */

/* 라이브러리의 rand() 대신 강의에서 만든 것과 같은 minstd 생성기를 쓴다.
 * 정렬을 부를 때마다 같은 시드로 되돌리므로, 같은 입력이면 비교·이동 횟수가
 * 매번 같다. 측정을 재현할 수 있어야 보고서의 숫자를 믿을 수 있다. */
#define QUICK_SEED 1

static int64_t randomState = QUICK_SEED;

static int64_t nextRandom(void) {
    randomState = randomState * 16807 % 2147483647;
    return randomState;
}

/* --- 파티션 ----------------------------------------------------------- */

/* a[lo..hi]를 a[lo]를 피벗으로 가른다. 피벗의 최종 자리를 돌려준다.
 * 피벗보다 **작은** 것만 왼쪽으로 보낸다. 같은 값은 오른쪽에 남는다. */
static size_t partition(SortCtx *c, size_t lo, size_t hi) {
    size_t i = lo;
    for (size_t j = lo + 1; j <= hi; j++) {
        if (sortCompareAt(c, j, lo) < 0) {
            i++;
            sortSwap(c, i, j);
        }
    }
    /* 피벗을 경계로 옮긴다. 이 자리는 확정이다. 멀리 뛰는 이 교환이
     * 같은 값의 순서를 뒤섞는다 — 퀵 정렬이 불안정한 이유다. */
    sortSwap(c, lo, i);
    return i;
}

static void quickSortRange(SortCtx *c, size_t lo, size_t hi, size_t depth, int randomPivot) {
    sortNoteDepth(c, depth);
    if (lo >= hi) {
        return; /* 원소 하나면 이미 정렬 */
    }
    if (randomPivot) {
        /* 구간 안에서 아무 자리나 골라 맨 앞으로 가져온다. 파티션은 그대로다.
         * 이 두 줄이 "정렬된 입력"이라는 최악을 없앤다. */
        size_t pick = lo + (size_t)(nextRandom() % (int64_t)(hi - lo + 1));
        if (pick != lo) {
            sortSwap(c, lo, pick);
        }
    }
    size_t p = partition(c, lo, hi);
    if (p > lo) {
        quickSortRange(c, lo, p - 1, depth + 1, randomPivot); /* 피벗 왼쪽 */
    }
    quickSortRange(c, p + 1, hi, depth + 1, randomPivot); /* 피벗 오른쪽 */
}

static void quickSortWith(void *base, size_t n, size_t size, SortCompare cmp,
                          SortStats *stats, int randomPivot) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    randomState = QUICK_SEED;
    quickSortRange(&c, 0, n - 1, 1, randomPivot);
    sortEnd(&c);
}

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    quickSortWith(base, n, size, cmp, stats, 1);
}

void quickSortFirstPivot(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    quickSortWith(base, n, size, cmp, stats, 0);
}
