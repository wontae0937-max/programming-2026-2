/* 힙 정렬 — 수업에서 다루지 않은 정렬 (보고서 2.3에 학습 과정을 적었다).
 *
 * 배열을 **최대 힙**으로 본다. 완전 이진 트리를 배열에 빈칸 없이 채운 것이라
 * 포인터가 필요 없다. 0부터 세면
 *
 *     부모 i 의 자식:  2i+1 (왼쪽),  2i+2 (오른쪽)
 *     자식 i 의 부모:  (i-1)/2
 *
 * 최대 힙은 "부모가 두 자식보다 작지 않다"는 규칙 하나만 지킨다. 그러면 가장
 * 큰 값은 늘 뿌리 a[0]에 있다. 정렬은 두 단계다.
 *
 *   1단계 · 힙 만들기   — 아래쪽 부모부터 거꾸로 올라가며 siftDown. O(n)
 *   2단계 · 하나씩 빼기 — 뿌리(최댓값)를 맨 뒤와 바꾸고, 힙을 한 칸 줄여
 *                        새 뿌리를 siftDown. 이것을 n-1번. O(n log n)
 *
 * 병합처럼 최악이 O(n log n)이고, 퀵처럼 temp 배열이 없다. 재귀도 없다.
 * 그 대신 내준 것은 안정성, 그리고 멀리 떨어진 칸을 오가는 메모리 접근이다.
 */
#include "sort.h"

#include "sortctx.h"

/* a[root]에서 시작해 힙 규칙을 다시 세운다. 힙의 크기는 end다 (a[0..end)).
 *
 * 교환을 되풀이하면 한 층 내려갈 때마다 이동이 3번이다. 대신 내려갈 원소를
 * tmp에 들고 있다가, 더 큰 자식을 한 칸씩 끌어올리고 마지막 빈자리에 한 번
 * 내려놓는다 (삽입 정렬이 원소를 미는 것과 같은 수). 한 층에 이동 1번이다. */
static void siftDown(SortCtx *c, size_t root, size_t end) {
    sortMove(c, c->tmp, sortElemAt(c, root));
    size_t hole = root;

    for (;;) {
        size_t child = 2 * hole + 1; /* 왼쪽 자식 */
        if (child >= end) {
            break; /* 잎에 닿았다 */
        }
        /* 오른쪽 자식이 있고 더 크면 그쪽으로 간다. 두 자식 중 큰 쪽이어야
         * 끌어올린 뒤에도 부모가 형제보다 크다. */
        if (child + 1 < end && sortCompareAt(c, child, child + 1) < 0) {
            child++;
        }
        /* 큰 자식이 들고 있는 원소보다 크지 않으면 여기가 제자리다. */
        if (sortCompareTmp(c, child) <= 0) {
            break;
        }
        sortMove(c, sortElemAt(c, hole), sortElemAt(c, child)); /* 자식을 끌어올린다 */
        hole = child;
    }
    sortMove(c, sortElemAt(c, hole), c->tmp);
}

void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 1단계: 자식이 있는 마지막 부모 (n/2 - 1)부터 뿌리까지 거꾸로. 잎은 이미
     * 혼자서 힙이므로 건너뛴다. 아래층이 넓고 얕아서 합이 O(n)으로 묶인다. */
    for (size_t i = n / 2; i-- > 0;) {
        siftDown(&c, i, n);
    }
    /* 2단계: 최댓값을 맨 뒤로 보내고 힙을 한 칸 줄인다. 이 교환이 같은 값의
     * 순서를 뒤섞는다 — 힙 정렬이 불안정한 이유다. */
    for (size_t end = n - 1; end > 0; end--) {
        sortSwap(&c, 0, end);
        siftDown(&c, 0, end);
    }
    sortEnd(&c);
}
