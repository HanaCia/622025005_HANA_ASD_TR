/*
 * menu.h
 * Menyatukan seluruh modul menjadi fitur aplikasi.
 * AppState menyimpan semua data yang sedang dipakai program.
 */
#ifndef MENU_H
#define MENU_H

#include "auth.h"
#include "parking.h"
#include "queue.h"
#include "stack.h"
#include "hashtable.h"
#include "bst.h"
#include "graph.h"

typedef struct {
    UserList   users;          /* array akun              (Topik 2) */
    User       currentUser;    /* akun yang sedang login            */
    ParkingLot lot;            /* array dinamis slot      (Topik 2) */
    Queue      carQueue;       /* antrean mobil           (Topik 4) */
    Stack      activityStack;  /* riwayat sesi dan undo   (Topik 4) */
    HashTable  plateTable;     /* plat -> indeks slot     (Topik 6) */
    BSTNode   *slotIndex;      /* ID slot -> indeks slot  (Topik 7) */
    Graph      mallMap;        /* denah mall              (Topik 8) */
    int        isActive;       /* 1 jika data sudah dimuat          */
} AppState;

int  initApp(AppState *app);
void shutdownApp(AppState *app);
void runStartMenu(AppState *app);

#endif
