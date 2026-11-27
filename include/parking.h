/*
 * parking.h
 * Data utama aplikasi: slot parkir yang disimpan dalam array dinamis.
 */
#ifndef PARKING_H
#define PARKING_H

#include "config.h"

typedef struct {
    char id[ID_LEN];            /* contoh "B1-A02"                         */
    char floor[FLOOR_LEN];      /* "B2", "B1", "P1", "P2"                   */
    char block;                 /* 'A' sampai 'D'                           */
    int  number;                /* nomor slot di dalam blok                 */
    int  floorLevel;            /* B2=1, B1=2, P1=3, P2=4 (kunci sort)      */
    int  isOccupied;            /* 0 = kosong, 1 = terisi                   */
    char plate[PLATE_LEN];      /* nomor polisi, "-" jika kosong            */
    char entryTime[TIME_LEN];   /* waktu masuk, "-" jika kosong             */
} ParkingSlot;

typedef struct {
    ParkingSlot *slots;         /* array dinamis hasil malloc */
    int count;                  /* jumlah slot yang terisi data */
    int capacity;               /* ukuran array yang dialokasikan */
} ParkingLot;

/* Memori dan file */
int  initParkingLot(ParkingLot *lot, int capacity);
void freeParkingLot(ParkingLot *lot);
int  loadParking(ParkingLot *lot, int *skipped);
int  saveParking(const ParkingLot *lot);
void createDefaultSlots(ParkingLot *lot);
ParkingSlot *copySlotArray(const ParkingLot *lot);

/* Validasi dan format */
const char *getFloorName(int order);
int  floorToLevel(const char *floor);
int  isValidFloor(const char *floor);
int  isValidBlock(char block);
void makeSlotId(char *output, const char *floor, char block, int number);
int  normalizePlate(const char *input, char *output);
int  isValidPlate(const char *plate);

/* Operasi slot */
int  addSlot(ParkingLot *lot, const char *floor, char block, int number);
int  deleteSlotAt(ParkingLot *lot, int index);
void occupySlot(ParkingSlot *slot, const char *plate, const char *time);
void releaseSlot(ParkingSlot *slot);

/* Perhitungan */
int  countEmptySlots(const ParkingLot *lot);
int  countSlotsOnFloor(const ParkingLot *lot, const char *floor);
int  countEmptyOnFloor(const ParkingLot *lot, const char *floor);
int  countSlotsInBlock(const ParkingLot *lot, const char *floor, char block);
int  countEmptyInBlock(const ParkingLot *lot, const char *floor, char block);
int  findFirstEmptyInBlock(const ParkingLot *lot, const char *floor, char block);

/* Tampilan */
void printSlotTable(const ParkingSlot *slots, int count);
void printSlotTableByIndex(const ParkingLot *lot, const int *indexes, int count);
void printSingleSlot(const ParkingSlot *slot);
void printFloorMap(const ParkingLot *lot);

#endif
