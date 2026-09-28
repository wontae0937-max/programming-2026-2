/* 유닛 테스트 — 외부 프레임워크 없이 표준 C만 쓴다.
 * 실행: make test-c
 *
 * 테스트도 공통 인터페이스로 쓴다. 구현 표(SORT_ALGORITHMS)를 훑으며
 * 모든 정렬에 같은 검사를 돌리므로, 정렬을 하나 더 넣어도 테스트는 그대로다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

static int checks = 0;
static int failures = 0;

static void report(const char *algo, const char *name, int ok) {
    checks++;
    if (ok) {
        printf("ok    %-14s %s\n", algo, name);
        return;
    }
    failures++;
    printf("FAIL  %-14s %s\n", algo, name);
}

/* --- int 배열 --------------------------------------------------------- */

static void expectSorted(const SortAlgorithm *algo, const char *name,
                         const int input[], const int want[], size_t n) {
    int a[32];
    SortStats stats;

    memcpy(a, input, n * sizeof(int));
    algo->sort(a, n, sizeof(a[0]), sortCompareInt, &stats);

    int ok = (n == 0) || memcmp(a, want, n * sizeof(int)) == 0;
    report(algo->name, name, ok);
    if (!ok) {
        printf("      got :");
        for (size_t i = 0; i < n; i++) {
            printf(" %d", a[i]);
        }
        printf("\n      want:");
        for (size_t i = 0; i < n; i++) {
            printf(" %d", want[i]);
        }
        printf("\n");
    }
}

/* --- 안정성 ----------------------------------------------------------- */

/* 원소와 비교 함수는 bench.h의 Record·recordCompare를 그대로 쓴다.
 * key로 정렬하고 tag에는 입력 순서를 담아 둔다. 정렬 뒤에도 같은 key끼리
 * tag가 오름차순이면 안정 정렬이다. */

static void expectStable(const SortAlgorithm *algo) {
    enum { N = 60 };
    Record a[N];
    SortStats stats;

    /* key는 0~4만 쓴다. 중복이 많아야 안정성이 드러난다. */
    for (int i = 0; i < N; i++) {
        a[i].key = (i * 7) % 5;
        a[i].tag = i;
    }
    algo->sort(a, N, sizeof(a[0]), recordCompare, &stats);

    int ok = 1;
    for (int i = 1; i < N; i++) {
        if (a[i - 1].key > a[i].key) {
            ok = 0; /* 정렬조차 안 됐다 */
        }
        if (a[i - 1].key == a[i].key && a[i - 1].tag > a[i].tag) {
            ok = 0; /* 같은 key인데 입력 순서가 뒤집혔다 */
        }
    }
    /* 구현 표의 stable 값이 실측과 맞는지 함께 본다. */
    report(algo->name, "안정성 (표의 stable 값과 일치)", ok == algo->stable);
}

/* --- 난수 배열을 qsort 결과와 맞춰 본다 ------------------------------- */

static void expectMatchesQsort(const SortAlgorithm *algo) {
    enum { N = 500 };
    int *a = malloc(N * sizeof(int));
    int *want = malloc(N * sizeof(int));
    SortStats stats;

    srand(20260901); /* 씨앗을 고정해 매번 같은 입력을 쓴다 */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100; /* 중복이 섞이도록 좁은 범위를 쓴다 */
        want[i] = a[i];
    }
    qsort(want, N, sizeof(want[0]), sortCompareInt);
    algo->sort(a, N, sizeof(a[0]), sortCompareInt, &stats);

    report(algo->name, "난수 500개가 qsort 결과와 같다",
           memcmp(a, want, N * sizeof(int)) == 0);
    free(a);
    free(want);
}

/* --- 크기를 바꿔 가며 --------------------------------------------------- */

/* 재귀 정렬은 경계(홀수 길이, 원소 한두 개 남은 구간)에서 틀리기 쉽다.
 * 2의 거듭제곱이 아닌 크기까지 전부 훑는다. 안정하다고 주장하는 정렬은
 * 안정성도 같은 자리에서 함께 본다. */
static void expectManySizes(const SortAlgorithm *algo) {
    enum { MAX_N = 200 };
    Record a[MAX_N];
    Record want[MAX_N];
    SortStats stats;
    int ok = 1;

    srand(20260902);
    for (size_t n = 0; n <= MAX_N; n++) {
        for (size_t i = 0; i < n; i++) {
            a[i].key = rand() % 20; /* 중복이 많은 입력 */
            a[i].tag = (int)i;
            want[i] = a[i];
        }
        /* qsort는 안정 정렬이 아니므로 tag까지 견줄 수 없다. key 순서는
         * qsort로 확인하고, tag 순서는 따로 본다. */
        qsort(want, n, sizeof(want[0]), recordCompare);
        algo->sort(a, n, sizeof(a[0]), recordCompare, &stats);

        for (size_t i = 0; i < n; i++) {
            if (a[i].key != want[i].key) {
                ok = 0;
            }
            if (algo->stable && i > 0 && a[i - 1].key == a[i].key &&
                a[i - 1].tag > a[i].tag) {
                ok = 0; /* 같은 key인데 입력 순서가 뒤집혔다 */
            }
        }
        if (!ok) {
            printf("      n = %zu에서 어긋났다\n", n);
            break;
        }
    }
    report(algo->name, algo->stable ? "n = 0..200 전부 정렬되고 안정하다"
                                    : "n = 0..200 전부 정렬된다", ok);
}

/* --- 측정값이 채워지는지 --------------------------------------------- */

static void expectStats(const SortAlgorithm *algo) {
    int a[] = {5, 1, 4, 2, 3};
    SortStats stats;

    algo->sort(a, 5, sizeof(a[0]), sortCompareInt, &stats);
    report(algo->name, "측정값이 채워진다",
           stats.compares > 0 && stats.moves > 0 &&
           stats.extraBytes >= sizeof(a[0]) && stats.maxDepth >= 1);
}

/* --- 강의 숫자 재현 ---------------------------------------------------- */

/* 강의 슬라이드(주제 03 · 04)에 나온 비교·이동 횟수를 그대로 내는지 본다.
 * 세는 규칙(교환 = 이동 3회, temp 왕복도 이동)이 강의와 같다는 확인이다. */
static void expectLectureCounts(const char *name,
                                void (*sort)(void *, size_t, size_t, SortCompare, SortStats *),
                                const int input[], size_t wantCompares, size_t wantMoves,
                                const char *label) {
    int a[10];
    SortStats stats;
    char title[96];

    memcpy(a, input, sizeof(a));
    sort(a, 10, sizeof(a[0]), sortCompareInt, &stats);
    snprintf(title, sizeof(title), "강의 %s: 비교 %zu · 이동 %zu", label, wantCompares, wantMoves);
    int ok = stats.compares == wantCompares && stats.moves == wantMoves;
    report(name, title, ok);
    if (!ok) {
        printf("      got : 비교 %zu · 이동 %zu\n", stats.compares, stats.moves);
    }
}

static void expectLectureNumbers(void) {
    static const int example[10] = {2, 8, 5, 9, 1, 10, 7, 6, 4, 3};
    static const int best[10] = {5, 1, 3, 4, 2, 8, 7, 6, 9, 10};
    static const int sorted[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    static const int reversed[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    /* 주제 03 · 입력을 바꾸면: 이동은 언제나 68회 */
    expectLectureCounts("mergeSort", mergeSort, example, 22, 68, "예제 배열");
    expectLectureCounts("mergeSort", mergeSort, sorted, 19, 68, "정렬됨");
    expectLectureCounts("mergeSort", mergeSort, reversed, 15, 68, "역순");
    /* 주제 04 · 입력을 바꾸면 (첫 원소 피벗) */
    expectLectureCounts("quickFirst", quickSortFirstPivot, best, 19, 9, "최선");
    expectLectureCounts("quickFirst", quickSortFirstPivot, example, 25, 21, "예제 배열");
    expectLectureCounts("quickFirst", quickSortFirstPivot, sorted, 45, 0, "정렬됨(최악)");
    expectLectureCounts("quickFirst", quickSortFirstPivot, reversed, 45, 15, "역순");
}

/* 랜덤 피벗이 정렬된 입력의 최악을 실제로 피하는지. n = 2000 정렬된 입력에서
 * 첫 원소 피벗은 n(n-1)/2 = 1,999,000회를 비교한다. 랜덤 피벗은 그 1/10도
 * 쓰지 않아야 한다 (기대값은 약 2n ln n ≈ 30,000). */
static void expectRandomPivotAvoidsWorstCase(void) {
    enum { N = 2000 };
    Record *a = malloc(N * sizeof(Record));
    SortStats first, random;

    makeInput(a, N, INPUT_SORTED, 1u);
    quickSortFirstPivot(a, N, sizeof(Record), recordCompare, &first);
    makeInput(a, N, INPUT_SORTED, 1u);
    quickSort(a, N, sizeof(Record), recordCompare, &random);
    report("quickSort", "정렬된 입력: 첫 원소 피벗은 n(n-1)/2회를 비교한다",
           first.compares == (size_t)N * (N - 1) / 2);
    report("quickSort", "정렬된 입력: 랜덤 피벗은 그 1/10 미만이다",
           random.compares * 10 < first.compares);
    free(a);
}

/* 랜덤 피벗이라도 시드를 고정했으므로 같은 입력이면 측정값이 같아야 한다. */
static void expectQuickReproducible(void) {
    enum { N = 500 };
    Record a[N];
    SortStats s1, s2;

    makeInput(a, N, INPUT_RANDOM, 7u);
    quickSort(a, N, sizeof(Record), recordCompare, &s1);
    makeInput(a, N, INPUT_RANDOM, 7u);
    quickSort(a, N, sizeof(Record), recordCompare, &s2);
    report("quickSort", "시드를 고정했으므로 두 번 돌려도 측정값이 같다",
           s1.compares == s2.compares && s1.moves == s2.moves);
}

/* 힙 정렬과 병합 정렬이 약속한 추가 메모리를 지키는지. */
static void expectMemoryClaims(void) {
    enum { N = 1000 };
    Record *a = malloc(N * sizeof(Record));
    SortStats s;

    makeInput(a, N, INPUT_RANDOM, 3u);
    heapSort(a, N, sizeof(Record), recordCompare, &s);
    report("heapSort", "추가 메모리가 원소 한 칸뿐이고 재귀가 없다",
           s.extraBytes == sizeof(Record) && s.maxDepth == 1);
    makeInput(a, N, INPUT_RANDOM, 3u);
    mergeSort(a, N, sizeof(Record), recordCompare, &s);
    report("mergeSort", "temp 배열로 원소 n칸을 더 쓴다",
           s.extraBytes == sizeof(Record) * (N + 1));
    free(a);
}

/* --- 측정 도구 자체 (bench.c) ----------------------------------------- */

static void expectInputShapes(void) {
    enum { N = 40 };
    Record a[N];
    int ok = 1;

    makeInput(a, N, INPUT_SORTED, 1u);
    if (!recordsSorted(a, N)) {
        ok = 0;
    }
    for (int i = 0; i < N; i++) {
        if (a[i].tag != i) {
            ok = 0; /* tag에는 입력 순서가 들어 있어야 한다 */
        }
    }
    report("bench", "makeInput(정렬됨)이 정렬된 입력을 만든다", ok);

    makeInput(a, N, INPUT_REVERSED, 1u);
    report("bench", "makeInput(역순)이 역순 입력을 만든다",
           N > 1 && !recordsSorted(a, N) && a[0].key > a[N - 1].key);

    makeInput(a, N, INPUT_FEW_UNIQUE, 1u);
    int distinct = 0;
    for (int i = 0; i < N; i++) {
        int seen = 0;
        for (int j = 0; j < i; j++) {
            if (a[j].key == a[i].key) {
                seen = 1;
            }
        }
        distinct += !seen;
    }
    report("bench", "makeInput(중복많음)의 서로 다른 key가 적다", distinct <= 8);
}

static void expectDetectorsCatchViolations(void) {
    Record a[4] = {{1, 0}, {1, 1}, {2, 2}, {2, 3}};

    report("bench", "정렬·안정 판정이 멀쩡한 배열을 통과시킨다",
           recordsSorted(a, 4) && recordsStable(a, 4));

    Record swapped[4] = {{1, 1}, {1, 0}, {2, 2}, {2, 3}};
    report("bench", "같은 key의 순서가 뒤집히면 안정하지 않다고 본다",
           recordsSorted(swapped, 4) && !recordsStable(swapped, 4));

    Record unsorted[4] = {{2, 0}, {1, 1}, {3, 2}, {4, 3}};
    report("bench", "정렬되지 않은 배열을 잡아낸다", !recordsSorted(unsorted, 4));

    /* 같은 key가 없으면 안정성을 판정할 수 없다. benchRun은 그때 -1을 준다. */
    Record distinct[4] = {{4, 0}, {3, 1}, {2, 2}, {1, 3}};
    BenchResult r = benchRun(&SORT_ALGORITHMS[0], distinct, 4, 1);
    report("bench", "같은 key가 없으면 안정성을 '판정 불가'(-1)로 둔다",
           !recordsHaveTies(a, 0) && recordsHaveTies(a, 4) && r.stable == -1);
}

static void expectBenchRun(const SortAlgorithm *algo) {
    enum { N = 300 };
    Record input[N];

    makeInput(input, N, INPUT_FEW_UNIQUE, 20260903u);
    BenchResult r = benchRun(algo, input, N, 2);

    report(algo->name, "benchRun이 정렬·안정·측정값을 채운다",
           r.sorted && r.stable == algo->stable && r.millis >= 0.0 &&
           r.stats.compares > 0 && r.n == N && r.algo == algo);
}

/* --- 전부 돌린다 ------------------------------------------------------ */

int main(void) {
    /* 비교 대상 셋과 실험용 변형을 한 줄로 세워 같은 검사를 돌린다. */
    const size_t total = SORT_ALGORITHM_COUNT + SORT_VARIANT_COUNT;
    for (size_t k = 0; k < total; k++) {
        const SortAlgorithm *algo = k < SORT_ALGORITHM_COUNT
                                        ? &SORT_ALGORITHMS[k]
                                        : &SORT_VARIANTS[k - SORT_ALGORITHM_COUNT];
        {
            const int a[] = {6, 8, 5, 9, 10, 1, 7, 2, 4, 3};
            const int want[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            expectSorted(algo, "섞인 배열", a, want, 10);
        }
        {
            const int a[] = {1, 2, 3, 4, 5};
            const int want[] = {1, 2, 3, 4, 5};
            expectSorted(algo, "이미 정렬된 배열", a, want, 5);
        }
        {
            const int a[] = {5, 4, 3, 2, 1};
            const int want[] = {1, 2, 3, 4, 5};
            expectSorted(algo, "역순 배열", a, want, 5);
        }
        {
            const int a[] = {3, 1, 3, 1, 2};
            const int want[] = {1, 1, 2, 3, 3};
            expectSorted(algo, "중복이 있는 배열", a, want, 5);
        }
        {
            const int a[] = {2, 2, 2, 2};
            const int want[] = {2, 2, 2, 2};
            expectSorted(algo, "모두 같은 값", a, want, 4);
        }
        {
            const int a[] = {42};
            const int want[] = {42};
            expectSorted(algo, "원소 하나", a, want, 1);
        }
        {
            /* n = 0이면 배열을 건드리지 않는다. */
            const int a[1] = {0};
            const int want[1] = {0};
            expectSorted(algo, "빈 배열", a, want, 0);
        }
        expectStable(algo);
        expectManySizes(algo);
        expectMatchesQsort(algo);
        expectStats(algo);
        expectBenchRun(algo);
        printf("\n");
    }

    expectLectureNumbers();
    expectRandomPivotAvoidsWorstCase();
    expectQuickReproducible();
    expectMemoryClaims();
    printf("\n");

    expectInputShapes();
    expectDetectorsCatchViolations();

    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
