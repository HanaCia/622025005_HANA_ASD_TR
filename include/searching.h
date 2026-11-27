/*
 * searching.h
 * Sequential Search dan Binary Search buatan sendiri (tanpa bsearch()).
 */
#ifndef SEARCHING_H
#define SEARCHING_H

#include "parking.h"

int sequentialSearchById(const ParkingSlot *array, int count, const char *id, int *steps);
int sequentialSearchEmpty(const ParkingSlot *array, int count, const char *floor, char block,
                          int *results, int maxResults, int *steps);
int binarySearchById(const ParkingSlot *sortedArray, int count, const char *id, int *steps);

#endif
