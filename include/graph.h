/*
 * graph.h
 * Denah parkir mall sebagai graph tak berarah (adjacency matrix).
 * Titik (node) : pintu masuk, persimpangan, ramp lantai, dan blok parkir.
 * Sisi (edge)  : jalan yang menghubungkan dua titik.
 * Slot parkir berada di dalam blok dan menjadi langkah terakhir rute.
 */
#ifndef GRAPH_H
#define GRAPH_H

#include "parking.h"

#define MAX_NODE        30
#define NODE_NAME_LEN   16

#define NODE_GATE       'G'   /* pintu masuk   */
#define NODE_JUNCTION   'J'   /* persimpangan  */
#define NODE_RAMP       'R'   /* ramp lantai   */
#define NODE_BLOCK      'B'   /* blok parkir   */

#define GATE_NAME       "GATE"

typedef struct {
    char name[NODE_NAME_LEN];
    char type;
    char floor[FLOOR_LEN];    /* "-" untuk gate dan persimpangan */
    char block;               /* '-' jika bukan blok */
} GraphNode;

typedef struct {
    GraphNode nodes[MAX_NODE];
    int adj[MAX_NODE][MAX_NODE];   /* 1 = terhubung, 0 = tidak */
    int nodeCount;
    int edgeCount;
} Graph;

void initGraph(Graph *graph);
int  addNode(Graph *graph, const char *name, char type, const char *floor, char block);
int  addEdge(Graph *graph, int from, int to);
int  findNodeByName(const Graph *graph, const char *name);
int  findBlockNode(const Graph *graph, const char *floor, char block);
void createDefaultGraph(Graph *graph);
int  loadGraph(Graph *graph);
int  saveGraph(const Graph *graph);

int  bfs(const Graph *graph, int start, int *visitOrder, int *parent);
int  buildPath(const int *parent, int target, int *path);
int  findRouteBFS(const Graph *graph, int start, int target, int *path, int *pathLength);
int  findNearestEmptyBlock(const Graph *graph, const ParkingLot *lot, int start,
                           int *path, int *pathLength, int showSteps);
void printRoute(const Graph *graph, const int *path, int pathLength, const char *slotId);
void dfsExplore(const Graph *graph, const ParkingLot *lot, int start);
void displayGraph(const Graph *graph, const ParkingLot *lot);

#endif
