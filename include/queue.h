/*
 * queue.h
 * Queue (antrean) berbasis linked list untuk kendaraan yang menunggu slot.
 * Prinsip FIFO: mobil yang datang lebih dulu dilayani lebih dulu.
 */
#ifndef QUEUE_H
#define QUEUE_H

#include "config.h"

typedef struct QueueNode {
    char plate[PLATE_LEN];
    char arrivalTime[TIME_LEN];
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;   /* mobil paling depan (dilayani berikutnya) */
    QueueNode *rear;    /* mobil paling belakang (baru datang)      */
    int size;
} Queue;

void initQueue(Queue *queue);
int  isQueueEmpty(const Queue *queue);
int  enqueue(Queue *queue, const char *plate, const char *arrivalTime);
int  dequeue(Queue *queue, char *plateOut, char *timeOut);
const QueueNode *peekQueue(const Queue *queue);
int  findInQueue(const Queue *queue, const char *plate);
void displayQueue(const Queue *queue);
void freeQueue(Queue *queue);
int  loadQueue(Queue *queue);
int  saveQueue(const Queue *queue);

#endif
