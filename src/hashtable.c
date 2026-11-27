/*
 * hashtable.c
 * Implementasi Hash Table dengan Separate Chaining.
 * Jika dua plat menghasilkan indeks bucket yang sama (collision),
 * keduanya disimpan dalam linked list pada bucket tersebut.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "display.h"

void initHashTable(HashTable *table)
{
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    table->count = 0;
}

/*
 * Fungsi hash polinomial sederhana:
 *   hash = hash * 31 + kode ASCII karakter, untuk setiap karakter
 *   indeks = hash % HASH_SIZE
 * Angka 31 membuat urutan huruf berpengaruh, jadi "AB12" dan "BA12"
 * menghasilkan nilai berbeda.
 */
unsigned int hashFunction(const char *key)
{
    unsigned int hash = 0;
    int i;
    for (i = 0; key[i] != '\0'; i++) {
        hash = hash * 31 + (unsigned char) key[i];
    }
    return hash % HASH_SIZE;
}

/* Menambah data. Jika plat sudah ada, indeks slotnya diperbarui. */
int hashInsert(HashTable *table, const char *plate, int slotIndex)
{
    unsigned int index = hashFunction(plate);
    HashNode *current = table->buckets[index];
    HashNode *node;

    while (current != NULL) {
        if (strcmp(current->plate, plate) == 0) {
            current->slotIndex = slotIndex;
            return 1;
        }
        current = current->next;
    }

    node = (HashNode *) malloc(sizeof(HashNode));
    if (node == NULL) {
        return 0;
    }
    snprintf(node->plate, PLATE_LEN, "%s", plate);
    node->slotIndex = slotIndex;
    node->next = table->buckets[index];   /* sisipkan di depan rantai */
    table->buckets[index] = node;
    table->count++;
    return 1;
}

/* Mengembalikan indeks slot atau -1. steps boleh NULL. */
int hashSearch(const HashTable *table, const char *plate, int *steps)
{
    const HashNode *current = table->buckets[hashFunction(plate)];

    while (current != NULL) {
        if (steps != NULL) (*steps)++;
        if (strcmp(current->plate, plate) == 0) {
            return current->slotIndex;
        }
        current = current->next;
    }
    return -1;
}

int hashDelete(HashTable *table, const char *plate)
{
    unsigned int index = hashFunction(plate);
    HashNode *current = table->buckets[index];
    HashNode *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->plate, plate) == 0) {
            if (previous == NULL) {
                table->buckets[index] = current->next;   /* node pertama rantai */
            } else {
                previous->next = current->next;          /* lewati node ini */
            }
            free(current);
            table->count--;
            return 1;
        }
        previous = current;
        current = current->next;
    }
    return 0;
}

/* Setelah slot dihapus dari array, indeks di belakangnya bergeser satu */
void hashShiftIndexes(HashTable *table, int removedIndex)
{
    HashNode *current;
    int i;

    for (i = 0; i < HASH_SIZE; i++) {
        for (current = table->buckets[i]; current != NULL; current = current->next) {
            if (current->slotIndex > removedIndex) {
                current->slotIndex--;
            }
        }
    }
}

void displayHashTable(const HashTable *table, const ParkingLot *lot)
{
    const HashNode *current;
    int i, chainLength;
    int usedBuckets = 0;
    int collisionBuckets = 0;

    printf("  Fungsi hash : hash = hash * 31 + ASCII,  indeks = hash %% %d\n\n", HASH_SIZE);
    for (i = 0; i < HASH_SIZE; i++) {
        printf("  %sBucket [%2d]%s : ", clr(CLR_CYAN), i, clr(CLR_RESET));
        current = table->buckets[i];
        chainLength = 0;
        if (current == NULL) {
            printf("%s(kosong)%s", clr(CLR_GRAY), clr(CLR_RESET));
        }
        while (current != NULL) {
            if (chainLength > 0) printf(" %s->%s ", clr(CLR_MAGENTA), clr(CLR_RESET));
            printf("%s%s%s(%s)", clr(CLR_YELLOW), current->plate, clr(CLR_RESET),
                   (current->slotIndex >= 0 && current->slotIndex < lot->count)
                       ? lot->slots[current->slotIndex].id : "?");
            chainLength++;
            current = current->next;
        }
        if (chainLength > 0) usedBuckets++;
        if (chainLength > 1) {
            collisionBuckets++;
            printf("  %s<- collision%s", clr(CLR_RED), clr(CLR_RESET));
        }
        printf("\n");
    }
    printf("\n  Jumlah data       : %d kendaraan\n", table->count);
    printf("  Bucket terpakai   : %d dari %d\n", usedBuckets, HASH_SIZE);
    printf("  Bucket collision  : %d (ditangani dengan separate chaining)\n", collisionBuckets);
}

void freeHashTable(HashTable *table)
{
    HashNode *current, *next;
    int i;

    for (i = 0; i < HASH_SIZE; i++) {
        current = table->buckets[i];
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
        table->buckets[i] = NULL;
    }
    table->count = 0;
}
