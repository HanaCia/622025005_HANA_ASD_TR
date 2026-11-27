/*
 * queue.c
 * Implementasi Queue dengan linked list.
 * enqueue menambah di belakang (rear), dequeue mengambil dari depan (front).
 * Format queue.txt per baris:  plat|waktuDatang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "parking.h"
#include "input.h"
#include "display.h"

void initQueue(Queue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

int isQueueEmpty(const Queue *queue)
{
    return queue->front == NULL;
}

int enqueue(Queue *queue, const char *plate, const char *arrivalTime)
{
    QueueNode *node = (QueueNode *) malloc(sizeof(QueueNode));
    if (node == NULL) {
        return 0;
    }
    snprintf(node->plate, PLATE_LEN, "%s", plate);
    snprintf(node->arrivalTime, TIME_LEN, "%s", arrivalTime);
    node->next = NULL;

    if (queue->rear == NULL) {          /* antrean masih kosong */
        queue->front = node;
        queue->rear = node;
    } else {                            /* sambungkan di belakang */
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
    return 1;
}

int dequeue(Queue *queue, char *plateOut, char *timeOut)
{
    QueueNode *removed;

    if (isQueueEmpty(queue)) {
        return 0;
    }
    removed = queue->front;
    if (plateOut != NULL) snprintf(plateOut, PLATE_LEN, "%s", removed->plate);
    if (timeOut != NULL) snprintf(timeOut, TIME_LEN, "%s", removed->arrivalTime);

    queue->front = removed->next;
    if (queue->front == NULL) {         /* antrean menjadi kosong */
        queue->rear = NULL;
    }
    free(removed);
    queue->size--;
    return 1;
}

const QueueNode *peekQueue(const Queue *queue)
{
    return queue->front;
}

/* Sequential search pada antrean. Mengembalikan posisi (mulai 1) atau 0. */
int findInQueue(const Queue *queue, const char *plate)
{
    const QueueNode *current = queue->front;
    int position = 1;

    while (current != NULL) {
        if (strcmp(current->plate, plate) == 0) {
            return position;
        }
        current = current->next;
        position++;
    }
    return 0;
}

void displayQueue(const Queue *queue)
{
    const QueueNode *current = queue->front;
    int position = 1;

    if (isQueueEmpty(queue)) {
        printInfo("Antrean kosong. Tidak ada kendaraan yang menunggu.");
        return;
    }

    printf("  %s+------+------------+------------------+%s\n", clr(CLR_BLUE), clr(CLR_RESET));
    printf("  | %s%-4s%s | %s%-10s%s | %s%-16s%s |\n",
           clr(CLR_YELLOW), "Urut", clr(CLR_RESET), clr(CLR_YELLOW), "No Polisi", clr(CLR_RESET),
           clr(CLR_YELLOW), "Waktu Datang", clr(CLR_RESET));
    printf("  %s+------+------------+------------------+%s\n", clr(CLR_BLUE), clr(CLR_RESET));
    while (current != NULL) {
        printf("  | %4d | %s%-10s%s | %-16s |", position, clr(CLR_YELLOW),
               current->plate, clr(CLR_RESET), current->arrivalTime);
        if (current == queue->front) printf(" %s<- DEPAN%s", clr(CLR_GREEN), clr(CLR_RESET));
        if (current == queue->rear && current != queue->front) {
            printf(" %s<- BELAKANG%s", clr(CLR_GRAY), clr(CLR_RESET));
        }
        printf("\n");
        current = current->next;
        position++;
    }
    printf("  %s+------+------------+------------------+%s\n", clr(CLR_BLUE), clr(CLR_RESET));
}

void freeQueue(Queue *queue)
{
    while (dequeue(queue, NULL, NULL)) {
        /* dequeue sudah membebaskan setiap node */
    }
}

int loadQueue(Queue *queue)
{
    FILE *file = fopen(FILE_QUEUE, "r");
    char line[LINE_LEN];
    char plate[PLATE_LEN];
    char arrival[TIME_LEN];

    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        trimText(line);
        if (sscanf(line, "%11[^|]|%19[^\n]", plate, arrival) == 2 &&
            isValidPlate(plate) && !findInQueue(queue, plate)) {
            enqueue(queue, plate, arrival);
        }
    }
    fclose(file);
    return 1;
}

int saveQueue(const Queue *queue)
{
    FILE *file = fopen(FILE_QUEUE, "w");
    const QueueNode *current = queue->front;

    if (file == NULL) {
        printError("Gagal menyimpan queue.txt.");
        return 0;
    }
    while (current != NULL) {
        fprintf(file, "%s|%s\n", current->plate, current->arrivalTime);
        current = current->next;
    }
    fclose(file);
    return 1;
}
