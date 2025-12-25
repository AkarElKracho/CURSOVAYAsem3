#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <limits.h>
#include <time.h>

#define MAX 20
typedef struct {
    int n;
    int capacity[MAX][MAX];
    int flow[MAX][MAX];
} Graph;
typedef struct {
    int source;
    int sink;
    int maxFlow;
    int ostatochek[MAX][MAX];
    int step;
    int foundPaths[MAX][MAX];
    int pathCount;
    int tekushayaFlows[MAX];
} FerrariFulkState;

void header() {
    printf("############################################################\n");
    printf("##/@\\===---*/          КУРСОВАЯ РАБОТА         \\*---===/@\\##\n");
    printf("##@0@===---*|         ЗАХАРОВ А.В. 24ВВВ1      |*---===@0@##\n");
    printf("##\\@/===---*\\      Алгоритм Форда-Фалкерсона   /*---===\\@/##\n");
    printf("############################################################\n\n");
}
int inputInt(int* x) {
    char buf[100];
    int value = 0;

    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    buf[strcspn(buf, "\n")] = '\0';

    if (buf[0] == '\0') return 0;

    for (int i = 0; buf[i]; i++) {
        if (buf[i] < '0' || buf[i] > '9') return 0;
        value = value * 10 + (buf[i] - '0');
    }

    *x = value;
    return 1;
}

void pause() {
    printf("\nНажмите Enter для продолжения...");
    while (getchar() != '\n');
}

void initGraph(Graph* g, int n) {
    g->n = n;
    memset(g->capacity, 0, sizeof(g->capacity));
    memset(g->flow, 0, sizeof(g->flow));
}

void initFerrariFulkState(FerrariFulkState* state, Graph* g, int source, int sink) {
    state->source = source;
    state->sink = sink;
    state->maxFlow = 0;
    state->step = 0;
    state->pathCount = 0;
    memset(state->foundPaths, 0, sizeof(state->foundPaths));
    memset(state->tekushayaFlows, 0, sizeof(state->tekushayaFlows));

    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            state->ostatochek[i][j] = g->capacity[i][j];
        }
    }
}

void inputGraph(Graph* g) {
    do {
        printf("Количество вершин (1..%d): ", MAX);
        if (!inputInt(&g->n) || g->n < 1 || g->n > MAX) {
            printf("Ошибка ввода!\n");
            continue;
        }
        break;
    } while (1);

    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            do {
                printf("C[%d][%d] = ", i + 1, j + 1);
                if (!inputInt(&g->capacity[i][j]) || g->capacity[i][j] < 0) {
                    printf("Ошибка ввода!\n");
                    continue;
                }
                break;
            } while (1);
        }
    }

    printf("Граф введен вручную\n");
}

void printMatrix(Graph* g) {
    if (g->n == 0) {
        printf("Граф не задан!\n");
        return;
    }

    printf("\nМатрица смежности:\n    ");
    for (int i = 0; i < g->n; i++)
        printf("%4d", i + 1);
    printf("\n");

    for (int i = 0; i < g->n; i++) {
        printf("%4d", i + 1);
        for (int j = 0; j < g->n; j++)
            printf("%4d", g->capacity[i][j]);
        printf("\n");
    }
}

void printEdges(Graph* g) {
    if (g->n == 0) {
        printf("Граф не задан!\n");
        return;
    }

    printf("\nДуги графа:\n");
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (g->capacity[i][j] > 0) {
                printf("%d -> %d : %d\n", i + 1, j + 1, g->capacity[i][j]);
            }
        }
    }
}

void loadGraph(Graph* g) {
    char filename[100];
    printf("Введите имя файла для загрузки: ");
    scanf("%s", filename);
    while (getchar() != '\n');

    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Ошибка открытия файла!\n");
        return;
    }

    fscanf(f, "%d", &g->n);
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            fscanf(f, "%d", &g->capacity[i][j]);
        }
    }

    fclose(f);
    printf("Граф успешно загружен из файла %s\n", filename);
}

void saveGraph(Graph* g) {
    if (g->n == 0) {
        printf("Граф не задан!\n");
        return;
    }

    char filename[100];
    printf("Введите имя файла для сохранения: ");
    scanf("%s", filename);
    while (getchar() != '\n');

    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Ошибка открытия файла!\n");
        return;
    }

    fprintf(f, "%d\n", g->n);
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            fprintf(f, "%d ", g->capacity[i][j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    printf("Граф успешно сохранен в файл %s\n", filename);
}

void generateGraph(Graph* g) {
    int maxCap;
    do {
        printf("Количество вершин (1..%d): ", MAX);
        if (!inputInt(&g->n) || g->n < 1 || g->n > MAX) {
            printf("Ошибка ввода!\n");
            continue;
        }
        break;
    } while (1);

    do {
        printf("Максимальная пропускная способность (>0): ");
        if (!inputInt(&maxCap) || maxCap <= 0) {
            printf("Ошибка ввода!\n");
            continue;
        }
        break;
    } while (1);

    memset(g->capacity, 0, sizeof(g->capacity));
    srand((unsigned)time(NULL));

    for (int i = 0; i < g->n - 1; i++) {
        g->capacity[i][i + 1] = rand() % maxCap + 1;
    }

    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (i != j && rand() % 3 == 0) {
                g->capacity[i][j] = rand() % maxCap + 1;
            }
        }
    }

    printf("Ориентированный граф сгенерирован\n");
}

void adaptMatrix(Graph* g, int source, int sink) {
    printf("\n=== АДАПТАЦИЯ МАТРИЦЫ ===\n");

    int original[MAX][MAX];
    memcpy(original, g->capacity, sizeof(original));

    printf("Удаляем входящие ребра в исток (%d):\n", source + 1);
    int removedToSource = 0;
    for (int i = 0; i < g->n; i++) {
        if (g->capacity[i][source] > 0) {
            printf("  Удалено ребро %d -> %d (емкость: %d)\n",
                i + 1, source + 1, g->capacity[i][source]);
            g->capacity[i][source] = 0;
            removedToSource++;
        }
    }
    if (removedToSource == 0) {
        printf("  Нет входящих ребер в исток\n");
    }

    printf("\nУдаляем исходящие ребра из стока (%d):\n", sink + 1);
    int removedFromSink = 0;
    for (int j = 0; j < g->n; j++) {
        if (g->capacity[sink][j] > 0) {
            printf("  Удалено ребро %d -> %d (емкость: %d)\n",
                sink + 1, j + 1, g->capacity[sink][j]);
            g->capacity[sink][j] = 0;
            removedFromSink++;
        }
    }
    if (removedFromSink == 0) {
        printf("  Нет исходящих ребер из стока\n");
    }

    printf("\nИзменения в матрице смежности:\n");
    printf("    ");
    for (int j = 0; j < g->n; j++) printf("%4d", j + 1);
    printf("\n");

    for (int i = 0; i < g->n; i++) {
        printf("%4d", i + 1);
        for (int j = 0; j < g->n; j++) {
            if (original[i][j] != g->capacity[i][j]) {
                if (g->capacity[i][j] == 0 && original[i][j] > 0) {
                    printf("  X ");
                }
                else {
                    printf("%4d", g->capacity[i][j]);
                }
            }
            else {
                printf("%4d", g->capacity[i][j]);
            }
        }
        printf("\n");
    }

    printf("\nВсего удалено: %d ребер\n", removedToSource + removedFromSink);
}

int bfs(FerrariFulkState* state, int parent[], int n) {
    int visited[MAX] = { 0 };
    int q[MAX], front = 0, rear = 0;

    q[rear++] = state->source;
    visited[state->source] = 1;
    parent[state->source] = -1;

    while (front < rear) {
        int u = q[front++];

        for (int v = 0; v < n; v++) {
            if (!visited[v] && state->ostatochek[u][v] > 0) {
                parent[v] = u;
                visited[v] = 1;

                if (v == state->sink) {
                    return 1;
                }

                q[rear++] = v;
            }
        }
    }

    return 0;
}

void reconstructPath(int parent[], int sink, int path[], int* pathLength) {
    *pathLength = 0;
    int current = sink;

    while (current != -1) {
        path[(*pathLength)++] = current;
        current = parent[current];
    }

    for (int i = 0; i < *pathLength / 2; i++) {
        int temp = path[i];
        path[i] = path[*pathLength - 1 - i];
        path[*pathLength - 1 - i] = temp;
    }
}

void saveResults(Graph* g, FerrariFulkState* state) {
    char filename[100];
    printf("Введите имя файла для сохранения результатов: ");
    scanf("%s", filename);
    while (getchar() != '\n');

    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Ошибка открытия файла!\n");
        return;
    }

    fprintf(f, "=== РЕЗУЛЬТАТЫ АЛГОРИТМА ФОРДА-ФАЛКЕРСОНА ===\n\n");
    fprintf(f, "Исток: %d\n", state->source + 1);
    fprintf(f, "Сток: %d\n", state->sink + 1);
    fprintf(f, "Максимальный поток: %d\n\n", state->maxFlow);

    fprintf(f, "Количество найденных увеличивающих путей: %d\n\n", state->pathCount);

    if (state->pathCount > 0) {
        fprintf(f, "Найденные увеличивающие пути:\n");
        for (int p = 0; p < state->pathCount; p++) {
            fprintf(f, "Путь %d (поток: %d): ", p + 1, state->tekushayaFlows[p]);
            int i = 0;
            while (state->foundPaths[p][i] != -1) {
                fprintf(f, "%d", state->foundPaths[p][i] + 1);
                if (state->foundPaths[p][i + 1] != -1) {
                    fprintf(f, " -> ");
                }
                i++;
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }

    fprintf(f, "Итоговые потоки по ребрам:\n");
    int totalEdges = 0;
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (g->capacity[i][j] > 0) {
                int flow = g->capacity[i][j] - state->ostatochek[i][j];
                if (flow > 0) {
                    fprintf(f, "%d -> %d: %d/%d\n",
                        i + 1, j + 1, flow, g->capacity[i][j]);
                    totalEdges++;
                }
            }
        }
    }

    if (totalEdges == 0) {
        fprintf(f, "Нет ненулевых потоков\n");
    }

    fprintf(f, "\nМинимальный разрез:\n");

    int visited[MAX] = { 0 };
    int q[MAX], front = 0, rear = 0;
    q[rear++] = state->source;
    visited[state->source] = 1;

    while (front < rear) {
        int u = q[front++];
        for (int v = 0; v < g->n; v++) {
            if (!visited[v] && state->ostatochek[u][v] > 0) {
                visited[v] = 1;
                q[rear++] = v;
            }
        }
    }

    fprintf(f, "S = {");
    int first = 1;
    for (int i = 0; i < g->n; i++) {
        if (visited[i]) {
            if (!first) fprintf(f, ", ");
            fprintf(f, "%d", i + 1);
            first = 0;
        }
    }
    fprintf(f, "}\n");

    fprintf(f, "T = {");
    first = 1;
    for (int i = 0; i < g->n; i++) {
        if (!visited[i]) {
            if (!first) fprintf(f, ", ");
            fprintf(f, "%d", i + 1);
            first = 0;
        }
    }
    fprintf(f, "}\n");

    fclose(f);
    printf("Результаты успешно сохранены в файл %s\n", filename);
}

void stepByStepWithAdaptation(Graph* g, int source, int sink) {
    printf("\n=== ПОШАГОВЫЙ АЛГОРИТМ ФОРДА-ФАЛКЕРСОНА С АДАПТАЦИЕЙ ===\n");
    Graph originalGraph = *g;
    printf("\n[ШАГ 1] АДАПТАЦИЯ МАТРИЦЫ\n");
    adaptMatrix(g, source, sink);
    printf("\nНажмите Enter для перехода к алгоритму...");
    getchar();
    printf("\n[ШАГ 2] ИНИЦИАЛИЗАЦИЯ АЛГОРИТМА\n");
    FerrariFulkState state;
    initFerrariFulkState(&state, g, source, sink);
    printf("Исток: %d\n", source + 1);
    printf("Сток: %d\n", sink + 1);
    printf("Исходная остаточная сеть создана\n");
    printf("\nНажмите Enter для начала поиска путей...");
    getchar();
    int parent[MAX];
    int step = 1;
    int continueAlgorithm = 1;

    while (continueAlgorithm) {
        printf("\n[ШАГ %d] ПОИСК УВЕЛИЧИВАЮЩЕГО ПУТИ\n", step + 2);
        memset(parent, -1, sizeof(parent));
        if (!bfs(&state, parent, g->n)) {
            printf("Увеличивающих путей больше не найдено!\n");
            break;
        }
        int path[MAX], pathLength;
        reconstructPath(parent, state.sink, path, &pathLength);

        int tekushayaFlow = INT_MAX;
        printf("Найден путь: ");
        for (int i = 0; i < pathLength; i++) {
            printf("%d", path[i] + 1);
            if (i < pathLength - 1) {
                printf(" -> ");
            }
        }
        printf("\n");

        printf("Ребра пути:\n");
        for (int i = 0; i < pathLength - 1; i++) {
            int u = path[i];
            int v = path[i + 1];
            printf("  %d -> %d: остаточная емкость = %d\n",
                u + 1, v + 1, state.ostatochek[u][v]);
            if (state.ostatochek[u][v] < tekushayaFlow) {
                tekushayaFlow = state.ostatochek[u][v];
            }
        }

        printf("Минимальная пропускная способность на пути: %d\n", tekushayaFlow);
        printf("\nОбновление остаточной сети:\n");

        for (int i = 0; i < pathLength - 1; i++) {
            int u = path[i];
            int v = path[i + 1];
            printf("  %d -> %d: %d - %d = %d\n",
                u + 1, v + 1,
                state.ostatochek[u][v], tekushayaFlow,
                state.ostatochek[u][v] - tekushayaFlow);

            printf("  %d -> %d: %d + %d = %d\n",
                v + 1, u + 1,
                state.ostatochek[v][u], tekushayaFlow,
                state.ostatochek[v][u] + tekushayaFlow);

            state.ostatochek[u][v] -= tekushayaFlow;
            state.ostatochek[v][u] += tekushayaFlow;
        }

        if (state.pathCount < MAX) {
            for (int i = 0; i < pathLength; i++) {
                state.foundPaths[state.pathCount][i] = path[i];
            }
            state.foundPaths[state.pathCount][pathLength] = -1;
            state.tekushayaFlows[state.pathCount] = tekushayaFlow;
            state.pathCount++;
        }

        state.maxFlow += tekushayaFlow;
        printf("\nТекущий максимальный поток: %d\n", state.maxFlow);
        step++;

        if (step < 10) {
            printf("\nПродолжить поиск путей? (1 - да, 0 - нет): ");
            if (!inputInt(&continueAlgorithm)) {
                continueAlgorithm = 0;
            }
        }
        else {
            printf("\nДостигнуто максимальное количество шагов.\n");
            continueAlgorithm = 0;
        }
    }

    printf("\n[ФИНАЛЬНЫЙ ШАГ] РЕЗУЛЬТАТЫ\n");
    printf("========================================\n");
    printf("Максимальный поток: %d\n", state.maxFlow);
    printf("Количество найденных увеличивающих путей: %d\n\n", state.pathCount);

    if (state.pathCount > 0) {
        printf("Все найденные пути:\n");
        for (int p = 0; p < state.pathCount; p++) {
            printf("Путь %d (поток %d): ", p + 1, state.tekushayaFlows[p]);
            int i = 0;
            while (state.foundPaths[p][i] != -1) {
                printf("%d", state.foundPaths[p][i] + 1);
                if (state.foundPaths[p][i + 1] != -1) {
                    printf(" -> ");
                }
                i++;
            }
            printf("\n");
        }
    }

    printf("\nПотоки по ребрам в адаптированном графе:\n");
    int hasFlow = 0;
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (g->capacity[i][j] > 0) {
                int flow = g->capacity[i][j] - state.ostatochek[i][j];
                if (flow > 0) {
                    printf("  %d -> %d: %d/%d\n",
                        i + 1, j + 1, flow, g->capacity[i][j]);
                    hasFlow = 1;
                }
            }
        }
    }
    if (!hasFlow) {
        printf("  Нет ненулевых потоков\n");
    }

    printf("\nСохранить результаты алгоритма? (1 - да, 0 - нет): ");
    int saveChoice;
    if (inputInt(&saveChoice) && saveChoice == 1) {
        saveResults(g, &state);
    }

    *g = originalGraph;
    printf("\nОригинальный граф восстановлен.\n");
}

int main() {
    setlocale(LC_ALL, "Russian");
    header();
    Graph g;
    initGraph(&g, 0);

    int choice, s, t;
    while (1) {
        printf("\n     МЕНЮ\n");
        printf("1. Сгенерировать граф\n");
        printf("2. Ввести граф вручную\n");
        printf("3. Загрузить из файла\n");
        printf("4. Сохранить граф в файл\n");
        printf("5. Показать матрицу\n");
        printf("6. Показать дуги\n");
        printf("7. Алгоритм Форда-Фалкерсона\n");
        printf("0. Выход\n");
        printf("Выбор: ");
        if (!inputInt(&choice)) {
            printf("Ошибка ввода! Введите число.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
        case 1:
            generateGraph(&g);
            break;
        case 2:
            inputGraph(&g);
            break;
        case 3:
            loadGraph(&g);
            break;
        case 4:
            saveGraph(&g);
            break;
        case 5:
            printMatrix(&g);
            break;
        case 6:
            printEdges(&g);
            break;
        case 7:
            if (g.n == 0) {
                printf("Граф не задан!\n");
                break;
            }

            do {
                printf("Введите исток (1..%d): ", g.n);
                if (!inputInt(&s) || s < 1 || s > g.n) {
                    printf("Ошибка ввода!\n");
                    continue;
                }
                break;
            } while (1);

            do {
                printf("Введите сток (1..%d, кроме %d): ", g.n, s);
                if (!inputInt(&t) || t < 1 || t > g.n || t == s) {
                    printf("Ошибка ввода!\n");
                    continue;
                }
                break;
            } while (1);

            stepByStepWithAdaptation(&g, s - 1, t - 1);
            break;

        case 0:
            printf("Выход из программы...\n");
            return 0;
        default:
            printf("Неверный выбор!\n");
        }

        if (choice != 0)
            pause();
    }

    return 0;
}