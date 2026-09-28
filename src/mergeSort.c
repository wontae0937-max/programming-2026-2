/* 병합 정렬 — 반으로 나누고, 각각 정렬한 뒤, 병합한다 (주제 03).
 *
 * 나누기는 공짜다. 가운데를 자르면 끝이다. 일은 돌아오는 길의 병합에서 한다.
 * 병합하려면 원소를 잠시 옮겨 둘 temp 배열이 필요하다. 이것이 병합 정렬이
 * 치르는 값 O(n)이다. 대신 어떤 입력이 와도 O(n log n)이고, 안정하다.
 *
 * 강의 코드(01_merge_sort/mergeSort.c)와 같은 모양으로 짰다. 원소를 temp로
 * 옮길 때 한 번, 다시 a로 돌려놓을 때 한 번 이동을 센다. 그래서 강의의 예제
 * 배열에서 비교 22회 · 이동 68회가 그대로 나온다 (tests/test_sort.c).
 */
#include "sort.h"

#include <stdlib.h>

#include "sortctx.h"

/* 정렬된 a[lo..mid]와 a[mid+1..hi]를 temp를 거쳐 병합한다. (양 끝 포함) */
static void merge(SortCtx *c, char *temp, size_t lo, size_t mid, size_t hi) {
    size_t i = lo;
    size_t j = mid + 1;
    size_t k = lo;

    while (i <= mid && j <= hi) {
        /* '<='라서 같은 값이면 왼쪽이 먼저 나간다. 이 한 글자가 안정성이다. */
        if (sortCompareAt(c, i, j) <= 0) {
            sortMove(c, temp + k * c->size, sortElemAt(c, i));
            i++;
        } else {
            sortMove(c, temp + k * c->size, sortElemAt(c, j));
            j++;
        }
        k++;
    }
    /* 한쪽이 먼저 바닥나면 나머지는 비교 없이 그대로 옮긴다. */
    while (i <= mid) {
        sortMove(c, temp + k * c->size, sortElemAt(c, i));
        i++;
        k++;
    }
    while (j <= hi) {
        sortMove(c, temp + k * c->size, sortElemAt(c, j));
        j++;
        k++;
    }
    /* temp에서 a로 돌려놓는다. 구간 전체가 늘 왕복하므로 이동 횟수는
     * 입력 모양과 무관하게 같다. */
    for (k = lo; k <= hi; k++) {
        sortMove(c, sortElemAt(c, k), temp + k * c->size);
    }
}

static void mergeSortRange(SortCtx *c, char *temp, size_t lo, size_t hi, size_t depth) {
    sortNoteDepth(c, depth);
    if (lo >= hi) {
        return; /* 원소 하나면 이미 정렬 */
    }
    size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(c, temp, lo, mid, depth + 1);
    mergeSortRange(c, temp, mid + 1, hi, depth + 1);
    merge(c, temp, lo, mid, hi);
}

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* temp는 한 번만 잡아 재귀 전체가 나눠 쓴다. 호출마다 잡으면 느려진다. */
    char *temp = (char *)malloc(n * size);
    if (temp == NULL) {
        sortEnd(&c);
        return;
    }
    if (stats != NULL) {
        stats->extraBytes += n * size;
    }
    mergeSortRange(&c, temp, 0, n - 1, 1);
    free(temp);
    sortEnd(&c);
}
