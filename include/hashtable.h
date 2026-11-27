/*
 * hashtable.h
 * Hash Table dengan Separate Chaining.
 * Kunci  : nomor polisi kendaraan yang sedang parkir.
 * Nilai  : indeks slot pada array parkir.
 * Dipakai untuk fitur "Cari Mobil Saya" dan "Kendaraan Keluar".
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "parking.h"

/* Bilangan prima kecil agar sebaran hash merata dan collision mudah diamati */
#define HASH_SIZE 13

typedef struct HashNode {
    char plate[PLATE_LEN];
    int  slotIndex;
    struct HashNode *next;   /* rantai untuk menangani collision */
} HashNode;

typedef struct {
    HashNode *buckets[HASH_SIZE];
    int count;
} HashTable;

void initHashTable(HashTable *table);
unsigned int hashFunction(const char *key);
int  hashInsert(HashTable *table, const char *plate, int slotIndex);
int  hashSearch(const HashTable *table, const char *plate, int *steps);
int  hashDelete(HashTable *table, const char *plate);
void hashShiftIndexes(HashTable *table, int removedIndex);
void displayHashTable(const HashTable *table, const ParkingLot *lot);
void freeHashTable(HashTable *table);

#endif
