/*
 * graph.c
 * Denah parkir, BFS untuk rute terpendek (berdasarkan jumlah edge),
 * dan DFS untuk menelusuri seluruh area yang terhubung.
 *
 * Format graph.txt:
 *   baris 1          : jumlah titik (N)
 *   N baris berikut  : nama|tipe|lantai|blok      (contoh: P1-A|B|P1|A)
 *   baris berikutnya : jumlah edge (M)
 *   M baris berikut  : namaTitik1|namaTitik2      (contoh: GATE|SIMPANG)
 */
#include <stdio.h>
#include <string.h>
#include "graph.h"
#include "input.h"
#include "display.h"

#define ROUTE_NODES_PER_LINE 4

void initGraph(Graph *graph)
{
    int i, j;
    graph->nodeCount = 0;
    graph->edgeCount = 0;
    for (i = 0; i < MAX_NODE; i++) {
        for (j = 0; j < MAX_NODE; j++) {
            graph->adj[i][j] = 0;
        }
    }
}

/* Menambah titik. Mengembalikan indeks titik atau -1 jika penuh. */
int addNode(Graph *graph, const char *name, char type, const char *floor, char block)
{
    GraphNode *node;

    if (graph->nodeCount >= MAX_NODE) {
        return -1;
    }
    node = &graph->nodes[graph->nodeCount];
    snprintf(node->name, NODE_NAME_LEN, "%s", name);
    node->type = type;
    snprintf(node->floor, FLOOR_LEN, "%s", floor);
    node->block = block;
    graph->nodeCount++;
    return graph->nodeCount - 1;
}

/* Graph tak berarah: matriks diisi di dua arah */
int addEdge(Graph *graph, int from, int to)
{
    if (from < 0 || to < 0 || from >= graph->nodeCount || to >= graph->nodeCount ||
        from == to || graph->adj[from][to]) {
        return 0;
    }
    graph->adj[from][to] = 1;
    graph->adj[to][from] = 1;
    graph->edgeCount++;
    return 1;
}

int findNodeByName(const Graph *graph, const char *name)
{
    int i;
    for (i = 0; i < graph->nodeCount; i++) {
        if (strcmp(graph->nodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int findBlockNode(const Graph *graph, const char *floor, char block)
{
    int i;
    for (i = 0; i < graph->nodeCount; i++) {
        if (graph->nodes[i].type == NODE_BLOCK && graph->nodes[i].block == block &&
            strcmp(graph->nodes[i].floor, floor) == 0) {
            return i;
        }
    }
    return -1;
}

static void connectByName(Graph *graph, const char *first, const char *second)
{
    addEdge(graph, findNodeByName(graph, first), findNodeByName(graph, second));
}

/*
 * Denah bawaan (22 titik). Setiap lantai berbentuk lingkaran:
 *   RAMP -- A -- B
 *    |           |
 *    C --------- D
 * GATE - SIMPANG, SIMPANG ke RAMP-P1 (naik) dan RAMP-B1 (turun),
 * RAMP-P1 - RAMP-P2, RAMP-B1 - RAMP-B2.
 */
void createDefaultGraph(Graph *graph)
{
    const char *floors[FLOOR_COUNT] = { "P1", "P2", "B1", "B2" };   /* urutan dari yang terdekat */
    char name[NODE_NAME_LEN], ramp[NODE_NAME_LEN];
    char a[NODE_NAME_LEN], b[NODE_NAME_LEN], c[NODE_NAME_LEN], d[NODE_NAME_LEN];
    char block;
    int f;

    initGraph(graph);
    addNode(graph, GATE_NAME, NODE_GATE, EMPTY_FIELD, '-');
    addNode(graph, "SIMPANG", NODE_JUNCTION, EMPTY_FIELD, '-');

    for (f = 0; f < FLOOR_COUNT; f++) {
        snprintf(ramp, NODE_NAME_LEN, "RAMP-%s", floors[f]);
        addNode(graph, ramp, NODE_RAMP, floors[f], '-');
        for (block = 'A'; block < 'A' + BLOCK_COUNT; block++) {
            snprintf(name, NODE_NAME_LEN, "%s-%c", floors[f], block);
            addNode(graph, name, NODE_BLOCK, floors[f], block);
        }
        snprintf(a, NODE_NAME_LEN, "%s-A", floors[f]);
        snprintf(b, NODE_NAME_LEN, "%s-B", floors[f]);
        snprintf(c, NODE_NAME_LEN, "%s-C", floors[f]);
        snprintf(d, NODE_NAME_LEN, "%s-D", floors[f]);
        connectByName(graph, ramp, a);
        connectByName(graph, ramp, c);
        connectByName(graph, a, b);
        connectByName(graph, c, d);
        connectByName(graph, b, d);
    }

    connectByName(graph, GATE_NAME, "SIMPANG");
    connectByName(graph, "SIMPANG", "RAMP-P1");
    connectByName(graph, "SIMPANG", "RAMP-B1");
    connectByName(graph, "RAMP-P1", "RAMP-P2");
    connectByName(graph, "RAMP-B1", "RAMP-B2");
}

/* Mengembalikan 1 jika file valid dan berhasil dibaca */
int loadGraph(Graph *graph)
{
    FILE *file = fopen(FILE_GRAPH, "r");
    char line[LINE_LEN];
    char name[NODE_NAME_LEN], first[NODE_NAME_LEN], second[NODE_NAME_LEN];
    char floor[FLOOR_LEN];
    char type, block;
    int nodeTotal = 0, edgeTotal = 0, i;

    if (file == NULL) {
        return 0;
    }
    initGraph(graph);

    if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%d", &nodeTotal) != 1 ||
        nodeTotal < 1 || nodeTotal > MAX_NODE) {
        fclose(file);
        return 0;
    }
    for (i = 0; i < nodeTotal; i++) {
        if (fgets(line, sizeof(line), file) == NULL) break;
        trimText(line);
        if (sscanf(line, "%15[^|]|%c|%3[^|]|%c", name, &type, floor, &block) != 4 ||
            findNodeByName(graph, name) != -1) {
            break;
        }
        addNode(graph, name, type, floor, block);
    }

    if (graph->nodeCount != nodeTotal || fgets(line, sizeof(line), file) == NULL ||
        sscanf(line, "%d", &edgeTotal) != 1 || edgeTotal < 0) {
        fclose(file);
        return 0;
    }
    for (i = 0; i < edgeTotal; i++) {
        if (fgets(line, sizeof(line), file) == NULL) break;
        trimText(line);
        if (sscanf(line, "%15[^|]|%15[^\n]", first, second) == 2) {
            connectByName(graph, first, second);
        }
    }
    fclose(file);

    /* Graph wajib memiliki titik GATE sebagai titik awal pencarian */
    return findNodeByName(graph, GATE_NAME) != -1 && graph->edgeCount > 0;
}

int saveGraph(const Graph *graph)
{
    FILE *file = fopen(FILE_GRAPH, "w");
    const GraphNode *node;
    int i, j;

    if (file == NULL) {
        printError("Gagal menyimpan graph.txt.");
        return 0;
    }
    fprintf(file, "%d\n", graph->nodeCount);
    for (i = 0; i < graph->nodeCount; i++) {
        node = &graph->nodes[i];
        fprintf(file, "%s|%c|%s|%c\n", node->name, node->type, node->floor, node->block);
    }
    fprintf(file, "%d\n", graph->edgeCount);
    for (i = 0; i < graph->nodeCount; i++) {
        for (j = i + 1; j < graph->nodeCount; j++) {   /* j > i agar edge tidak ditulis dua kali */
            if (graph->adj[i][j]) {
                fprintf(file, "%s|%s\n", graph->nodes[i].name, graph->nodes[j].name);
            }
        }
    }
    fclose(file);
    return 1;
}

/*
 * BFS (Breadth First Search) memakai queue berbasis array.
 * Titik dikunjungi lapis demi lapis: semua titik berjarak 1 edge,
 * lalu semua titik berjarak 2 edge, dan seterusnya.
 * visitOrder : urutan titik yang dikunjungi
 * parent     : titik asal sebelum mencapai titik ini (untuk menyusun rute)
 * Mengembalikan jumlah titik yang dikunjungi.
 */
int bfs(const Graph *graph, int start, int *visitOrder, int *parent)
{
    int visited[MAX_NODE] = { 0 };
    int queue[MAX_NODE];
    int front = 0, rear = 0, count = 0;
    int current, next;

    for (next = 0; next < MAX_NODE; next++) {
        parent[next] = -1;
    }
    visited[start] = 1;
    queue[rear++] = start;               /* enqueue titik awal */

    while (front < rear) {
        current = queue[front++];        /* dequeue */
        visitOrder[count++] = current;
        for (next = 0; next < graph->nodeCount; next++) {
            if (graph->adj[current][next] && !visited[next]) {
                visited[next] = 1;
                parent[next] = current;
                queue[rear++] = next;    /* enqueue tetangga */
            }
        }
    }
    return count;
}

/* Menyusun rute dari parent[]: mundur dari target ke awal, lalu dibalik */
int buildPath(const int *parent, int target, int *path)
{
    int reversed[MAX_NODE];
    int length = 0, i;
    int current = target;

    while (current != -1 && length < MAX_NODE) {
        reversed[length++] = current;
        current = parent[current];
    }
    for (i = 0; i < length; i++) {
        path[i] = reversed[length - 1 - i];
    }
    return length;
}

/* Rute terpendek dari start ke target. Mengembalikan 1 jika ada jalur. */
int findRouteBFS(const Graph *graph, int start, int target, int *path, int *pathLength)
{
    int visitOrder[MAX_NODE], parent[MAX_NODE];
    int visitedCount, i;

    visitedCount = bfs(graph, start, visitOrder, parent);
    for (i = 0; i < visitedCount; i++) {
        if (visitOrder[i] == target) {
            *pathLength = buildPath(parent, target, path);
            return 1;
        }
    }
    return 0;
}

/*
 * Mencari blok terdekat yang masih punya slot kosong.
 * Karena BFS mengunjungi titik berdasarkan jarak, blok kosong PERTAMA
 * yang ditemui pada visitOrder pasti yang paling dekat dari start.
 * Mengembalikan indeks titik blok, atau -1 jika tidak ada.
 */
int findNearestEmptyBlock(const Graph *graph, const ParkingLot *lot, int start,
                          int *path, int *pathLength, int showSteps)
{
    int visitOrder[MAX_NODE], parent[MAX_NODE];
    int visitedCount, i, node;
    const GraphNode *info;

    visitedCount = bfs(graph, start, visitOrder, parent);
    if (showSteps) {
        printf("  %sBFS memeriksa titik:%s ", clr(CLR_CYAN), clr(CLR_RESET));
    }
    for (i = 0; i < visitedCount; i++) {
        node = visitOrder[i];
        info = &graph->nodes[node];
        if (showSteps) {
            printf("%s%s", (i > 0) ? ", " : "", info->name);
        }
        if (info->type == NODE_BLOCK && countEmptyInBlock(lot, info->floor, info->block) > 0) {
            if (showSteps) {
                printf("  %s<- ada slot kosong%s\n", clr(CLR_GREEN), clr(CLR_RESET));
            }
            *pathLength = buildPath(parent, node, path);
            return node;
        }
    }
    if (showSteps) {
        printf("\n");
    }
    return -1;
}

void printRoute(const Graph *graph, const int *path, int pathLength, const char *slotId)
{
    int i;

    printf("\n  %sRUTE PARKIR%s\n  ", clr(CLR_MAGENTA), clr(CLR_RESET));
    for (i = 0; i < pathLength; i++) {
        if (i > 0 && i % ROUTE_NODES_PER_LINE == 0) {
            printf("\n  ");
        }
        printf("%s%s%s -> ", clr(i == 0 ? CLR_YELLOW : CLR_CYAN),
               graph->nodes[path[i]].name, clr(CLR_RESET));
    }
    printf("%s[SLOT %s]%s\n", clr(CLR_GREEN), slotId, clr(CLR_RESET));
    printf("  Jarak : %d edge dari pintu masuk sampai blok, lalu menuju slot.\n",
           pathLength > 0 ? pathLength - 1 : 0);
}

/* DFS rekursif: masuk sedalam mungkin ke satu cabang sebelum pindah cabang */
static void dfsVisit(const Graph *graph, const ParkingLot *lot, int current, int *visited,
                     int depth, int *visitNumber, int *firstEmptyBlock)
{
    const GraphNode *info = &graph->nodes[current];
    int next, empty, total;

    visited[current] = 1;
    (*visitNumber)++;
    printf("  %2d. %*s%s", *visitNumber, depth * 2, "", info->name);

    if (info->type == NODE_BLOCK) {
        empty = countEmptyInBlock(lot, info->floor, info->block);
        total = countSlotsInBlock(lot, info->floor, info->block);
        printf("  %s[kosong %d/%d]%s", clr(empty > 0 ? CLR_GREEN : CLR_RED), empty, total,
               clr(CLR_RESET));
        if (empty > 0 && *firstEmptyBlock == -1) {
            *firstEmptyBlock = current;
        }
    }
    printf("\n");

    for (next = 0; next < graph->nodeCount; next++) {
        if (graph->adj[current][next] && !visited[next]) {
            dfsVisit(graph, lot, next, visited, depth + 1, visitNumber, firstEmptyBlock);
        }
    }
}

void dfsExplore(const Graph *graph, const ParkingLot *lot, int start)
{
    int visited[MAX_NODE] = { 0 };
    int visitNumber = 0;
    int firstEmptyBlock = -1;
    int i, blockCount = 0, emptyBlockCount = 0;
    const GraphNode *info;

    printInfo("DFS menelusuri satu cabang sampai ujung, lalu kembali (backtrack).");
    printInfo("Indentasi menunjukkan kedalaman penelusuran dari pintu masuk.\n");
    dfsVisit(graph, lot, start, visited, 0, &visitNumber, &firstEmptyBlock);

    for (i = 0; i < graph->nodeCount; i++) {
        info = &graph->nodes[i];
        if (visited[i] && info->type == NODE_BLOCK) {
            blockCount++;
            if (countEmptyInBlock(lot, info->floor, info->block) > 0) emptyBlockCount++;
        }
    }

    printf("\n  Titik terjangkau dari %s : %d dari %d titik\n", graph->nodes[start].name,
           visitNumber, graph->nodeCount);
    printf("  Blok terjangkau             : %d (%d blok masih punya slot kosong)\n",
           blockCount, emptyBlockCount);
    if (firstEmptyBlock != -1) {
        printf("  Blok kosong pertama versi DFS: %s%s%s (belum tentu yang terdekat,\n"
               "  rute terdekat dihitung dengan BFS)\n",
               clr(CLR_YELLOW), graph->nodes[firstEmptyBlock].name, clr(CLR_RESET));
    }
    if (visitNumber < graph->nodeCount) {
        printWarning("Ada titik yang tidak terhubung dengan pintu masuk.");
    }
}

/* Menampilkan graph dalam bentuk daftar tetangga (adjacency list) */
void displayGraph(const Graph *graph, const ParkingLot *lot)
{
    const GraphNode *info;
    int i, j, first;

    printf("  Jumlah titik: %d   Jumlah edge: %d\n\n", graph->nodeCount, graph->edgeCount);
    for (i = 0; i < graph->nodeCount; i++) {
        info = &graph->nodes[i];
        printf("  %s[%2d] %-8s%s", clr(CLR_CYAN), i, info->name, clr(CLR_RESET));
        if (info->type == NODE_BLOCK) {
            printf(" %s(%d kosong)%s", clr(CLR_GREEN),
                   countEmptyInBlock(lot, info->floor, info->block), clr(CLR_RESET));
        } else {
            printf("           ");
        }
        printf(" : ");
        first = 1;
        for (j = 0; j < graph->nodeCount; j++) {
            if (graph->adj[i][j]) {
                printf("%s%s", first ? "" : ", ", graph->nodes[j].name);
                first = 0;
            }
        }
        printf("\n");
    }
}
