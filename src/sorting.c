/*
 * sorting.c
 * Implementasi Bubble Sort dan Selection Sort tanpa qsort().
 * Pengurutan selalu dilakukan pada SALINAN array, bukan array utama,
 * agar indeks yang disimpan di Hash Table dan BST tetap benar.
 */
#include <string.h>
#include "sorting.h"

/*
 * Membandingkan dua slot berdasarkan kunci.
 * Hasil < 0 : a di depan b,  0 : sama,  > 0 : a di belakang b.
 * Jika nilai kunci sama, ID dipakai sebagai pembanding kedua.
 */
int compareSlots(const ParkingSlot *a, const ParkingSlot *b, int key)
{
    int result = 0;

    if (key == SORT_BY_FLOOR) {
        result = a->floorLevel - b->floorLevel;
    } else if (key == SORT_BY_NUMBER) {
        result = a->number - b->number;
    }
    if (result == 0) {
        result = strcmp(a->id, b->id);
    }
    return result;
}

/* Bernilai 1 jika a seharusnya berada di belakang b sesuai arah urutan */
static int isOutOfOrder(const ParkingSlot *a, const ParkingSlot *b, int key, int order)
{
    int result = compareSlots(a, b, key);
    return (order == ORDER_ASC) ? (result > 0) : (result < 0);
}

/* Menukar isi dua slot memakai pointer */
static void swapSlots(ParkingSlot *a, ParkingSlot *b)
{
    ParkingSlot temp = *a;
    *a = *b;
    *b = temp;
}

/*
 * Bubble Sort: bandingkan dua elemen yang bersebelahan, tukar jika urutannya
 * salah. Setiap putaran membawa elemen "terbesar" ke ujung kanan.
 * Jika satu putaran penuh tidak ada pertukaran, data sudah terurut (berhenti).
 */
void bubbleSort(ParkingSlot *array, int count, int key, int order, SortStats *stats)
{
    int pass, j, swapped;

    stats->comparisons = 0;
    stats->swaps = 0;
    for (pass = 0; pass < count - 1; pass++) {
        swapped = 0;
        for (j = 0; j < count - 1 - pass; j++) {
            stats->comparisons++;
            if (isOutOfOrder(&array[j], &array[j + 1], key, order)) {
                swapSlots(&array[j], &array[j + 1]);
                stats->swaps++;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

/*
 * Selection Sort: pada setiap putaran i, cari elemen yang seharusnya berada
 * paling depan dari sisa data (indeks i sampai akhir), lalu tukar ke posisi i.
 * Jumlah pertukaran paling banyak count - 1 kali.
 */
void selectionSort(ParkingSlot *array, int count, int key, int order, SortStats *stats)
{
    int i, j, selected;

    stats->comparisons = 0;
    stats->swaps = 0;
    for (i = 0; i < count - 1; i++) {
        selected = i;
        for (j = i + 1; j < count; j++) {
            stats->comparisons++;
            if (isOutOfOrder(&array[selected], &array[j], key, order)) {
                selected = j;
            }
        }
        if (selected != i) {
            swapSlots(&array[i], &array[selected]);
            stats->swaps++;
        }
    }
}

const char *sortKeyText(int key)
{
    if (key == SORT_BY_FLOOR) return "Lantai";
    if (key == SORT_BY_NUMBER) return "Nomor Slot";
    return "ID Slot";
}

const char *sortOrderText(int order)
{
    return (order == ORDER_DESC) ? "Descending" : "Ascending";
}
