/* 정렬 구현들이 함께 쓰는 작업 문맥 — 구현 전용 헤더.
 *
 * sort.h가 바깥에 보이는 인터페이스라면, 이쪽은 구현끼리만 쓰는 도구다.
 * 정렬을 부르는 코드(main.c, bench.c, 테스트)는 이 파일을 include하지 않는다.
 *
 * 헤더에 드러나는 이름은 파일 밖에서도 보이므로 sort- 를 붙여 둔다.
 * 한 파일 안에서만 쓰는 함수는 static으로 감춘다.
 */
#ifndef SORTCTX_H
#define SORTCTX_H

#include <stddef.h>

#include "sort.h"

/* 정렬 함수의 인자 다섯 개를 한 덩어리로 들고 다닌다. tmp는 원소 하나를
 * 잠시 담아 두는 자리다. 병합 정렬은 여기에 더해 temp 배열을 따로 잡는다. */
typedef struct SortCtx {
    char *base;
    size_t size;
    SortCompare cmp;
    SortStats *stats;
    char *tmp;
} SortCtx;

/* 정렬들이 똑같이 하는 앞처리·뒤처리. 정렬할 것이 없으면 sortBegin이 0을
 * 돌려주고, 그때는 sortEnd를 부르지 않는다. */
int sortBegin(SortCtx *c, void *base, size_t n, size_t size,
              SortCompare cmp, SortStats *stats);
void sortEnd(SortCtx *c);

char *sortElemAt(const SortCtx *c, size_t i);
int sortCompareAt(SortCtx *c, size_t i, size_t j); /* a[i]와 a[j] */
int sortCompareTmp(SortCtx *c, size_t i);          /* a[i]와 tmp에 담아 둔 원소 */
void sortMove(SortCtx *c, void *dst, const void *src);
void sortSwap(SortCtx *c, size_t i, size_t j);

/* 재귀 깊이를 기록한다. 병합·퀵이 쓴다. */
void sortNoteDepth(SortCtx *c, size_t depth);

#endif /* SORTCTX_H */
