/*
 * sorting.h
 * Bubble Sort dan Selection Sort buatan sendiri untuk data slot parkir.
 */
#ifndef SORTING_H
#define SORTING_H

#include "parking.h"

#define SORT_BY_ID      1
#define SORT_BY_FLOOR   2
#define SORT_BY_NUMBER  3

#define ORDER_ASC       1
#define ORDER_DESC      2

/* Statistik untuk analisis: berapa kali membandingkan dan menukar data */
typedef struct {
    long comparisons;
    long swaps;
} SortStats;

int  compareSlots(const ParkingSlot *a, const ParkingSlot *b, int key);
void bubbleSort(ParkingSlot *array, int count, int key, int order, SortStats *stats);
void selectionSort(ParkingSlot *array, int count, int key, int order, SortStats *stats);
const char *sortKeyText(int key);
const char *sortOrderText(int order);

#endif
