/*
 * searching.c
 * Sequential Search: memeriksa data satu per satu dari awal, bisa dipakai
 *                    pada data yang belum terurut.
 * Binary Search    : membagi dua rentang pencarian setiap langkah, WAJIB
 *                    pada data yang sudah terurut dengan kunci yang sama (ID).
 * Parameter steps menghitung jumlah perbandingan untuk analisis.
 */
#include <string.h>
#include "searching.h"

/* Mengembalikan indeks slot dengan ID tersebut, atau -1 jika tidak ada */
int sequentialSearchById(const ParkingSlot *array, int count, const char *id, int *steps)
{
    int i;
    for (i = 0; i < count; i++) {
        (*steps)++;
        if (strcmp(array[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Mencari SEMUA slot kosong yang cocok dengan lokasi.
 * floor kosong ("") berarti semua lantai, block '*' berarti semua blok.
 * Indeks hasil disimpan ke array results. Mengembalikan jumlah hasil.
 */
int sequentialSearchEmpty(const ParkingSlot *array, int count, const char *floor, char block,
                          int *results, int maxResults, int *steps)
{
    int i;
    int found = 0;

    for (i = 0; i < count && found < maxResults; i++) {
        (*steps)++;
        if (array[i].isOccupied) continue;
        if (floor[0] != '\0' && strcmp(array[i].floor, floor) != 0) continue;
        if (block != '*' && array[i].block != block) continue;
        results[found] = i;
        found++;
    }
    return found;
}

int binarySearchById(const ParkingSlot *sortedArray, int count, const char *id, int *steps)
{
    int low = 0;
    int high = count - 1;
    int mid, result;

    while (low <= high) {
        mid = low + (high - low) / 2;      /* titik tengah rentang */
        (*steps)++;
        result = strcmp(sortedArray[mid].id, id);
        if (result == 0) {
            return mid;                    /* ketemu */
        } else if (result < 0) {
            low = mid + 1;                 /* cari di separuh kanan */
        } else {
            high = mid - 1;                /* cari di separuh kiri  */
        }
    }
    return -1;
}
