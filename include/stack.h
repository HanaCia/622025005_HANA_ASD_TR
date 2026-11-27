/*
 * stack.h
 * Stack (tumpukan) berbasis linked list untuk menyimpan aktivitas terbaru.
 * Prinsip LIFO: aktivitas terakhir berada paling atas, sehingga cocok
 * untuk fitur undo (membatalkan perubahan terakhir lebih dulu).
 */
#ifndef STACK_H
#define STACK_H

#include "config.h"

/* Jenis aktivitas yang mengubah status slot */
#define ACT_PARK          1   /* mobil dari antrean masuk ke slot */
#define ACT_LEAVE         2   /* mobil keluar dari slot           */
#define ACT_SET_EMPTY     3   /* admin mengubah slot jadi kosong  */
#define ACT_SET_OCCUPIED  4   /* admin mengubah slot jadi terisi  */

typedef struct {
    int  action;
    char slotId[ID_LEN];
    char plate[PLATE_LEN];
    char time[TIME_LEN];
    char username[NAME_LEN];
    /* Kondisi slot SEBELUM aktivitas, dipakai untuk undo */
    int  prevOccupied;
    char prevPlate[PLATE_LEN];
    char prevEntryTime[TIME_LEN];
} Activity;

typedef struct StackNode {
    Activity data;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int size;
} Stack;

void initStack(Stack *stack);
int  isStackEmpty(const Stack *stack);
int  push(Stack *stack, const Activity *activity);
int  pop(Stack *stack, Activity *activityOut);
const Activity *peekStack(const Stack *stack);
void freeStack(Stack *stack);

#endif
