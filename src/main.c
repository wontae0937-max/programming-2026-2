/* 정렬 비교 — 병합 / 퀵 / 힙.
 *
 *   make run                 사람이 읽는 비교 표 (실험 셋 전부)
 *   ./src/main.out --csv     실험 1·2를 CSV로 (tools/plot.py가 쓴다)
 *   ./src/main.out --pivot   실험 3: 첫 원소 피벗 vs 랜덤 피벗 CSV
 *   ./src/main.out --dups    실험 4: 중복 정도를 바꿔 가며 CSV
 *
 * 부르는 쪽은 정렬 이름을 하나도 적지 않는다. 구현 표(SORT_ALGORITHMS)를
 * 훑을 뿐이다. 무엇을 잴지도 아래 SPECS 한 곳에만 적는다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

#define SEED 20260929u

/* --- 무엇을 잴 것인가 -------------------------------------------------- */

typedef struct Spec {
    const char *scope; /* kinds: 입력 모양별 · growth: n을 키우며 */
    InputKind kind;
    size_t n;
    int reps;
} Spec;

static const Spec SPECS[] = {
    /* 실험 1 · 입력 모양별 */
    {"kinds", INPUT_RANDOM, 20000, 5},
    {"kinds", INPUT_SORTED, 20000, 5},
    {"kinds", INPUT_REVERSED, 20000, 5},
    {"kinds", INPUT_FEW_UNIQUE, 20000, 5},
    /* 실험 2 · n을 4배씩 키우며. 백만 개면 배열이 8 MB라 캐시를 넘친다. */
    {"growth", INPUT_RANDOM, 1000, 50},
    {"growth", INPUT_RANDOM, 4000, 20},
    {"growth", INPUT_RANDOM, 16000, 10},
    {"growth", INPUT_RANDOM, 64000, 5},
    {"growth", INPUT_RANDOM, 256000, 5},
    {"growth", INPUT_RANDOM, 1024000, 5},
};

static const size_t SPEC_COUNT = sizeof(SPECS) / sizeof(SPECS[0]);

/* 실험 3 · 피벗. 첫 원소 피벗은 정렬된 입력에서 O(n^2)이라 n을 작게 둔다. */
static const size_t PIVOT_N = 4000;

/* 실험 4 · 중복. 서로 다른 key의 개수를 바꿔 가며 잰다. */
static const size_t DUPS_N = 20000;
static const int DUPS_DISTINCT[] = {1, 2, 8, 64, 512, 4096, 20000};

/* 측정 결과 한 줄을 받아 가는 곳. 표로 찍을지 CSV로 찍을지만 다르다 —
 * 정렬을 함수 포인터로 갈아 끼웠듯, 출력도 같은 수를 쓴다. */
typedef void (*RowSink)(const Spec *spec, const BenchResult *r);

/* SPECS를 훑으며 측정하고, 한 줄이 나올 때마다 sink에 넘긴다.
 * onSpec은 줄을 찍기 전에 불린다 (표가 소제목을 낼 자리). */
static void measureAll(RowSink sink, void (*onSpec)(const Spec *spec)) {
    for (size_t s = 0; s < SPEC_COUNT; s++) {
        const Spec *spec = &SPECS[s];
        Record *input = (Record *)malloc(spec->n * sizeof(Record));
        if (input == NULL) {
            return;
        }
        makeInput(input, spec->n, spec->kind, SEED);
        if (onSpec != NULL) {
            onSpec(spec);
        }
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, spec->n, spec->reps);
            sink(spec, &r);
        }
        free(input);
    }
}

/* --- 사람이 읽는 표 ---------------------------------------------------- */

#define ROW_FORMAT "%-14s %9.3f %12zu %12zu %10zu B %8zu %5s %6s\n"
#define ROW_HEADER "알고리즘        시간(ms)         비교         이동       메모리 재귀깊이  정렬 안정성\n"
#define ROW_RULE   "-------------------------------------------------------------------------------------\n"

static void tableRow(const Spec *spec, const BenchResult *r) {
    (void)spec;
    printf(ROW_FORMAT, r->algo->name, r->millis, r->stats.compares, r->stats.moves,
           r->stats.extraBytes, r->stats.maxDepth, r->sorted ? "yes" : "NO!",
           r->stable < 0 ? "-" : (r->stable ? "yes" : "no"));
}

static void tableSpecHeader(const Spec *spec) {
    static const char *lastScope = NULL;

    if (lastScope == NULL || strcmp(lastScope, spec->scope) != 0) {
        if (strcmp(spec->scope, "kinds") == 0) {
            printf("[실험 1] 입력 모양별 비교 (n = %zu, %d회 중 최솟값)\n", spec->n, spec->reps);
        } else {
            printf("\n[실험 2] n을 키우며 (무작위 입력)\n");
        }
        lastScope = spec->scope;
    }
    if (strcmp(spec->scope, "kinds") == 0) {
        printf("\n[%s]\n", inputKindName(spec->kind));
    } else {
        printf("\n[n = %zu, %d회 중 최솟값]\n", spec->n, spec->reps);
    }
    printf("%s%s", ROW_HEADER, ROW_RULE);
}

/* 구현 표가 뭐라고 주장하는지 먼저 보여 준다. 아래 측정과 견줘 보라고. */
static void printDeclarations(void) {
    printf("구현 표 (SortAlgorithm이 주장하는 값)\n");
    /* 한글은 터미널에서 두 칸을 쓴다. %-14s는 바이트를 세므로 머리글은 손으로 맞춘다. */
    printf("알고리즘       시간복잡도     메모리     안정성\n");
    printf("%s", ROW_RULE);
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];
        printf("%-14s %-14s %-10s %s\n", algo->name, algo->timeComplexity,
               algo->spaceComplexity, algo->stable ? "stable" : "unstable");
    }
    printf("\n");
}

/* --- 실험 3 · 피벗 ----------------------------------------------------- */

typedef void (*PivotSink)(InputKind kind, const BenchResult *r);

static void measurePivot(PivotSink sink) {
    const SortAlgorithm *pair[2] = {&SORT_VARIANTS[0], NULL};
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        if (strcmp(SORT_ALGORITHMS[k].name, "quickSort") == 0) {
            pair[1] = &SORT_ALGORITHMS[k];
        }
    }
    if (pair[1] == NULL) {
        return;
    }
    Record *input = (Record *)malloc(PIVOT_N * sizeof(Record));
    if (input == NULL) {
        return;
    }
    for (int kind = 0; kind < INPUT_KIND_COUNT; kind++) {
        makeInput(input, PIVOT_N, (InputKind)kind, SEED);
        for (int p = 0; p < 2; p++) {
            BenchResult r = benchRun(pair[p], input, PIVOT_N, 3);
            sink((InputKind)kind, &r);
        }
    }
    free(input);
}

static void pivotTableRow(InputKind kind, const BenchResult *r) {
    static int lastKind = -1;
    if ((int)kind != lastKind) {
        printf("\n[%s]\n%s%s", inputKindName(kind), ROW_HEADER, ROW_RULE);
        lastKind = (int)kind;
    }
    tableRow(NULL, r);
}

/* --- 실험 4 · 중복 ----------------------------------------------------- */

typedef void (*DupsSink)(int distinct, const BenchResult *r);

static void measureDups(DupsSink sink) {
    const size_t count = sizeof(DUPS_DISTINCT) / sizeof(DUPS_DISTINCT[0]);
    Record *input = (Record *)malloc(DUPS_N * sizeof(Record));
    if (input == NULL) {
        return;
    }
    for (size_t d = 0; d < count; d++) {
        makeInputDistinct(input, DUPS_N, DUPS_DISTINCT[d], SEED);
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, DUPS_N, 3);
            sink(DUPS_DISTINCT[d], &r);
        }
    }
    free(input);
}

static void dupsTableRow(int distinct, const BenchResult *r) {
    static int last = -1;
    if (distinct != last) {
        printf("\n[서로 다른 key %d개]\n%s%s", distinct, ROW_HEADER, ROW_RULE);
        last = distinct;
    }
    tableRow(NULL, r);
}

static void reportTable(void) {
    printf("=== 정렬 비교: 병합 · 퀵 · 힙 ===\n");
    printf("원소는 (key, tag) %zu바이트. key로 정렬하고 tag로 안정성을 본다.\n\n",
           sizeof(Record));
    printDeclarations();
    measureAll(tableRow, tableSpecHeader);

    printf("\n[실험 3] 퀵 정렬의 피벗: 첫 원소(quickFirst) vs 랜덤(quickSort), n = %zu\n",
           PIVOT_N);
    measurePivot(pivotTableRow);

    printf("\n[실험 4] 중복이 많아지면 (n = %zu, 무작위)\n", DUPS_N);
    measureDups(dupsTableRow);

    printf("\n읽는 법\n");
    printf("  시간   : 같은 기계에서만 견준다. 비교·이동 횟수가 더 믿을 만하다.\n");
    printf("  메모리 : 입력 배열 밖에 잡은 바이트. 병합만 temp 배열로 n칸을 더 쓴다.\n");
    printf("           퀵은 그 대신 재귀 깊이만큼 스택을 쓴다. 힙은 둘 다 없다.\n");
    printf("  안정성 : 표의 주장이 아니라 tag 순서로 실측한 값이다. '-'는 같은 key가\n");
    printf("           하나도 없어 판정할 수 없다는 뜻이다 (그때는 어떤 정렬이든 안정해 보인다).\n");
}

/* --- 기계가 읽는 CSV --------------------------------------------------- */

/* CSV에는 ASCII 키를 쓴다. 표에 찍는 한글 이름(inputKindName)과 따로 둔다. */
static const char *inputKindKey(InputKind kind) {
    switch (kind) {
        case INPUT_RANDOM:     return "random";
        case INPUT_SORTED:     return "sorted";
        case INPUT_REVERSED:   return "reversed";
        case INPUT_FEW_UNIQUE: return "few-unique";
        default:               return "unknown";
    }
}

#define CSV_TAIL "%s,%.3f,%zu,%zu,%zu,%zu,%d,%d\n"
#define CSV_TAIL_ARGS(r) (r)->algo->name, (r)->millis, (r)->stats.compares, \
    (r)->stats.moves, (r)->stats.extraBytes, (r)->stats.maxDepth, (r)->sorted, (r)->stable
#define CSV_TAIL_HEADER "algo,millis,compares,moves,extraBytes,maxDepth,sorted,stable\n"

static void csvRow(const Spec *spec, const BenchResult *r) {
    printf("%s,%s,%zu," CSV_TAIL, spec->scope, inputKindKey(spec->kind), spec->n,
           CSV_TAIL_ARGS(r));
}

static void pivotCsvRow(InputKind kind, const BenchResult *r) {
    printf("%s,%zu," CSV_TAIL, inputKindKey(kind), r->n, CSV_TAIL_ARGS(r));
}

static void dupsCsvRow(int distinct, const BenchResult *r) {
    printf("%d,%zu," CSV_TAIL, distinct, r->n, CSV_TAIL_ARGS(r));
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--csv") == 0) {
        printf("scope,input,n," CSV_TAIL_HEADER);
        measureAll(csvRow, NULL);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--pivot") == 0) {
        printf("input,n," CSV_TAIL_HEADER);
        measurePivot(pivotCsvRow);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--dups") == 0) {
        printf("distinct,n," CSV_TAIL_HEADER);
        measureDups(dupsCsvRow);
        return 0;
    }
    reportTable();
    return 0;
}
