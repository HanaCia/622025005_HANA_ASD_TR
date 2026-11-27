/*
 * parking.c
 * Pengelolaan array slot parkir: alokasi memori, baca/tulis parking.txt,
 * tambah/hapus slot, ubah status, perhitungan, dan tampilan tabel.
 *
 * Format parking.txt per baris:
 *   ID|lantai|blok|nomor|status|plat|waktuMasuk
 *   contoh: P1-A01|P1|A|1|1|H1234AB|2026-11-27 10:15
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parking.h"
#include "searching.h"
#include "input.h"
#include "display.h"

/* Urutan lantai dari bawah ke atas. Indeks + 1 = floorLevel. */
static const char *FLOOR_NAMES[FLOOR_COUNT] = { "B2", "B1", "P1", "P2" };

/* Data contoh agar demo tidak dimulai dari parkiran yang benar-benar kosong */
#define DEMO_COUNT 5
static const char *DEMO_SLOTS[DEMO_COUNT]  = { "P1-A01", "P1-A02", "P1-A03", "P1-C01", "B1-A01" };
static const char *DEMO_PLATES[DEMO_COUNT] = { "H1234AB", "H5678CD", "AB1122EF", "B9001XY", "K4455GH" };

/* ===================== Memori ===================== */

int initParkingLot(ParkingLot *lot, int capacity)
{
    lot->slots = (ParkingSlot *) malloc(sizeof(ParkingSlot) * (size_t) capacity);
    lot->count = 0;
    if (lot->slots == NULL) {
        lot->capacity = 0;
        return 0;
    }
    lot->capacity = capacity;
    return 1;
}

void freeParkingLot(ParkingLot *lot)
{
    free(lot->slots);
    lot->slots = NULL;
    lot->count = 0;
    lot->capacity = 0;
}

/* Membuat salinan array (malloc). Pemanggil WAJIB memanggil free(). */
ParkingSlot *copySlotArray(const ParkingLot *lot)
{
    ParkingSlot *copy;
    int i;

    if (lot->count == 0) {
        return NULL;
    }
    copy = (ParkingSlot *) malloc(sizeof(ParkingSlot) * (size_t) lot->count);
    if (copy == NULL) {
        return NULL;
    }
    for (i = 0; i < lot->count; i++) {
        copy[i] = lot->slots[i];
    }
    return copy;
}

/* ===================== Validasi dan format ===================== */

const char *getFloorName(int order)
{
    if (order < 0 || order >= FLOOR_COUNT) {
        return EMPTY_FIELD;
    }
    return FLOOR_NAMES[order];
}

/* "B2" -> 1, "B1" -> 2, "P1" -> 3, "P2" -> 4, tidak valid -> 0 */
int floorToLevel(const char *floor)
{
    int i;
    for (i = 0; i < FLOOR_COUNT; i++) {
        if (strcmp(floor, FLOOR_NAMES[i]) == 0) {
            return i + 1;
        }
    }
    return 0;
}

int isValidFloor(const char *floor)
{
    return floorToLevel(floor) != 0;
}

int isValidBlock(char block)
{
    return block >= 'A' && block < 'A' + BLOCK_COUNT;
}

/* Contoh: ("B1", 'A', 2) -> "B1-A02" */
void makeSlotId(char *output, const char *floor, char block, int number)
{
    snprintf(output, ID_LEN, "%.3s-%c%02d", floor, block, number);
}

/* Menghapus spasi dan mengubah ke huruf besar: "h 1234 ab" -> "H1234AB" */
int normalizePlate(const char *input, char *output)
{
    int i;
    int j = 0;

    for (i = 0; input[i] != '\0'; i++) {
        if (isspace((unsigned char) input[i])) {
            continue;
        }
        if (j >= PLATE_LEN - 1) {      /* terlalu panjang */
            output[0] = '\0';
            return 0;
        }
        output[j] = (char) toupper((unsigned char) input[i]);
        j++;
    }
    output[j] = '\0';
    return j > 0;
}

/*
 * Pola plat Indonesia: 1-2 huruf, 1-4 angka, 0-3 huruf.
 * Contoh valid: H1234AB, B1XY, AB12.
 */
int isValidPlate(const char *plate)
{
    int i = 0;
    int prefix = 0;
    int digits = 0;
    int suffix = 0;

    while (isalpha((unsigned char) plate[i])) { prefix++; i++; }
    while (isdigit((unsigned char) plate[i])) { digits++; i++; }
    while (isalpha((unsigned char) plate[i])) { suffix++; i++; }

    return plate[i] == '\0' &&
           prefix >= 1 && prefix <= PLATE_MAX_PREFIX &&
           digits >= 1 && digits <= PLATE_MAX_DIGIT &&
           suffix <= PLATE_MAX_SUFFIX;
}

/* ===================== Operasi slot ===================== */

/* Menambah slot di akhir array. Mengembalikan indeks baru atau -1 bila penuh. */
int addSlot(ParkingLot *lot, const char *floor, char block, int number)
{
    ParkingSlot *slot;

    if (lot->count >= lot->capacity) {
        return -1;
    }
    slot = &lot->slots[lot->count];
    makeSlotId(slot->id, floor, block, number);
    snprintf(slot->floor, FLOOR_LEN, "%s", floor);
    slot->block = block;
    slot->number = number;
    slot->floorLevel = floorToLevel(floor);
    releaseSlot(slot);   /* status awal: kosong */
    lot->count++;
    return lot->count - 1;
}

/* Menghapus slot pada indeks tertentu dengan menggeser elemen setelahnya */
int deleteSlotAt(ParkingLot *lot, int index)
{
    int i;

    if (index < 0 || index >= lot->count) {
        return 0;
    }
    for (i = index; i < lot->count - 1; i++) {
        lot->slots[i] = lot->slots[i + 1];
    }
    lot->count--;
    return 1;
}

/* Pointer ke slot dipakai agar perubahan langsung mengenai data asli */
void occupySlot(ParkingSlot *slot, const char *plate, const char *time)
{
    slot->isOccupied = 1;
    snprintf(slot->plate, PLATE_LEN, "%s", plate);
    snprintf(slot->entryTime, TIME_LEN, "%s", time);
}

void releaseSlot(ParkingSlot *slot)
{
    slot->isOccupied = 0;
    strcpy(slot->plate, EMPTY_FIELD);
    strcpy(slot->entryTime, EMPTY_FIELD);
}

/* ===================== File ===================== */

/*
 * Mengembalikan 1 bila minimal satu slot berhasil dibaca.
 * *skipped diisi jumlah baris yang tidak valid (dilewati).
 */
int loadParking(ParkingLot *lot, int *skipped)
{
    FILE *file = fopen(FILE_PARKING, "r");
    char line[LINE_LEN];
    ParkingSlot temp;
    int steps = 0;

    *skipped = 0;
    if (file == NULL) {
        return 0;
    }

    lot->count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        trimText(line);
        if (line[0] == '\0') {
            continue;
        }
        /* Angka di format (%15, %3, %11, %19) = ukuran buffer dikurangi 1 */
        if (lot->count >= lot->capacity ||
            sscanf(line, "%15[^|]|%3[^|]|%c|%d|%d|%11[^|]|%19[^\n]",
                   temp.id, temp.floor, &temp.block, &temp.number,
                   &temp.isOccupied, temp.plate, temp.entryTime) != 7 ||
            !isValidFloor(temp.floor) || !isValidBlock(temp.block) ||
            temp.number < 1 || temp.number > MAX_SLOT_NUMBER) {
            (*skipped)++;
            continue;
        }

        makeSlotId(temp.id, temp.floor, temp.block, temp.number);   /* ID dibuat ulang agar konsisten */
        if (sequentialSearchById(lot->slots, lot->count, temp.id, &steps) != -1) {
            (*skipped)++;   /* ID ganda diabaikan */
            continue;
        }

        temp.floorLevel = floorToLevel(temp.floor);
        temp.isOccupied = (temp.isOccupied == 1 && isValidPlate(temp.plate));
        if (!temp.isOccupied) {
            strcpy(temp.plate, EMPTY_FIELD);
            strcpy(temp.entryTime, EMPTY_FIELD);
        }
        lot->slots[lot->count] = temp;
        lot->count++;
    }
    fclose(file);
    return lot->count > 0;
}

int saveParking(const ParkingLot *lot)
{
    FILE *file = fopen(FILE_PARKING, "w");
    const ParkingSlot *slot;
    int i;

    if (file == NULL) {
        printError("Gagal menyimpan parking.txt. Pastikan folder data/ ada.");
        return 0;
    }
    for (i = 0; i < lot->count; i++) {
        slot = &lot->slots[i];
        fprintf(file, "%s|%s|%c|%d|%d|%s|%s\n", slot->id, slot->floor, slot->block,
                slot->number, slot->isOccupied, slot->plate, slot->entryTime);
    }
    fclose(file);
    return 1;
}

/* Membuat 4 lantai x 4 blok x 3 slot = 48 slot, lalu mengisi beberapa slot contoh */
void createDefaultSlots(ParkingLot *lot)
{
    char now[TIME_LEN];
    int floor, number, i, index, steps;
    char block;

    lot->count = 0;
    for (floor = 0; floor < FLOOR_COUNT; floor++) {
        for (block = 'A'; block < 'A' + BLOCK_COUNT; block++) {
            for (number = 1; number <= SLOT_PER_BLOCK; number++) {
                addSlot(lot, FLOOR_NAMES[floor], block, number);
            }
        }
    }

    getCurrentTime(now, TIME_LEN);
    for (i = 0; i < DEMO_COUNT; i++) {
        steps = 0;
        index = sequentialSearchById(lot->slots, lot->count, DEMO_SLOTS[i], &steps);
        if (index != -1) {
            occupySlot(&lot->slots[index], DEMO_PLATES[i], now);
        }
    }
}

/* ===================== Perhitungan ===================== */

int countEmptySlots(const ParkingLot *lot)
{
    int i, total = 0;
    for (i = 0; i < lot->count; i++) {
        if (!lot->slots[i].isOccupied) total++;
    }
    return total;
}

int countSlotsOnFloor(const ParkingLot *lot, const char *floor)
{
    int i, total = 0;
    for (i = 0; i < lot->count; i++) {
        if (strcmp(lot->slots[i].floor, floor) == 0) total++;
    }
    return total;
}

int countEmptyOnFloor(const ParkingLot *lot, const char *floor)
{
    int i, total = 0;
    for (i = 0; i < lot->count; i++) {
        if (strcmp(lot->slots[i].floor, floor) == 0 && !lot->slots[i].isOccupied) total++;
    }
    return total;
}

int countSlotsInBlock(const ParkingLot *lot, const char *floor, char block)
{
    int i, total = 0;
    for (i = 0; i < lot->count; i++) {
        if (strcmp(lot->slots[i].floor, floor) == 0 && lot->slots[i].block == block) total++;
    }
    return total;
}

int countEmptyInBlock(const ParkingLot *lot, const char *floor, char block)
{
    int i, total = 0;
    for (i = 0; i < lot->count; i++) {
        if (strcmp(lot->slots[i].floor, floor) == 0 && lot->slots[i].block == block &&
            !lot->slots[i].isOccupied) {
            total++;
        }
    }
    return total;
}

/* Mencari slot kosong dengan nomor terkecil di blok tertentu. -1 jika tidak ada. */
int findFirstEmptyInBlock(const ParkingLot *lot, const char *floor, char block)
{
    int i;
    int best = -1;

    for (i = 0; i < lot->count; i++) {
        const ParkingSlot *slot = &lot->slots[i];
        if (strcmp(slot->floor, floor) == 0 && slot->block == block && !slot->isOccupied) {
            if (best == -1 || slot->number < lot->slots[best].number) {
                best = i;
            }
        }
    }
    return best;
}

/* ===================== Tampilan ===================== */

static void printTableBorder(void)
{
    printf("  %s+-----+---------+----+------+------+--------+------------+------------------+%s\n",
           clr(CLR_BLUE), clr(CLR_RESET));
}

static void printTableHeader(void)
{
    printTableBorder();
    printf("  | %s%-3s%s | %s%-7s%s | %s%-2s%s | %s%-4s%s | %s%-4s%s | %s%-6s%s | %s%-10s%s | %s%-16s%s |\n",
           clr(CLR_YELLOW), "No", clr(CLR_RESET), clr(CLR_YELLOW), "ID Slot", clr(CLR_RESET),
           clr(CLR_YELLOW), "Lt", clr(CLR_RESET), clr(CLR_YELLOW), "Blok", clr(CLR_RESET),
           clr(CLR_YELLOW), "Slot", clr(CLR_RESET), clr(CLR_YELLOW), "Status", clr(CLR_RESET),
           clr(CLR_YELLOW), "No Polisi", clr(CLR_RESET), clr(CLR_YELLOW), "Waktu Masuk", clr(CLR_RESET));
    printTableBorder();
}

static void printTableRow(int number, const ParkingSlot *slot)
{
    printf("  | %3d | %-7s | %-2s | %-4c | %4d | ", number, slot->id, slot->floor,
           slot->block, slot->number);
    printStatusText(slot->isOccupied);
    printf(" | %-10s | %-16s |\n", slot->plate, slot->entryTime);
}

void printSlotTable(const ParkingSlot *slots, int count)
{
    int i;
    printTableHeader();
    for (i = 0; i < count; i++) {
        printTableRow(i + 1, &slots[i]);
    }
    printTableBorder();
}

void printSlotTableByIndex(const ParkingLot *lot, const int *indexes, int count)
{
    int i;
    printTableHeader();
    for (i = 0; i < count; i++) {
        printTableRow(i + 1, &lot->slots[indexes[i]]);
    }
    printTableBorder();
}

void printSingleSlot(const ParkingSlot *slot)
{
    printTableHeader();
    printTableRow(1, slot);
    printTableBorder();
}

/* Peta status per lantai, dari lantai paling atas ke paling bawah */
void printFloorMap(const ParkingLot *lot)
{
    int floor, i;
    char block;
    const ParkingSlot *slot;

    printf("\n  Keterangan: %s[nn]%s kosong   %s[nn]%s terisi\n\n",
           clr(CLR_GREEN), clr(CLR_RESET), clr(CLR_RED), clr(CLR_RESET));

    for (floor = FLOOR_COUNT - 1; floor >= 0; floor--) {
        printf("  %s%s%s |", clr(CLR_CYAN), FLOOR_NAMES[floor], clr(CLR_RESET));
        for (block = 'A'; block < 'A' + BLOCK_COUNT; block++) {
            printf(" %c", block);
            for (i = 0; i < lot->count; i++) {
                slot = &lot->slots[i];
                if (strcmp(slot->floor, FLOOR_NAMES[floor]) == 0 && slot->block == block) {
                    printf("%s[%02d]%s", clr(slot->isOccupied ? CLR_RED : CLR_GREEN),
                           slot->number, clr(CLR_RESET));
                }
            }
            printf(" ");
        }
        printf("\n");
    }
}
