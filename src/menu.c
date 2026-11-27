/*
 * menu.c
 * Alur aplikasi dan fitur-fitur menu. Setiap fitur ditulis sebagai satu
 * fungsi agar tanggung jawabnya jelas. Di sinilah struktur data saling
 * terhubung, misalnya:
 *   Kendaraan masuk  -> Queue
 *   Proses antrean   -> Queue + BFS (Graph) + Hash Table + Stack + File
 *   Kendaraan keluar -> Hash Table + Stack + File
 *   Cari slot        -> Sequential Search / Sorting + Binary Search / BST
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "display.h"
#include "input.h"
#include "sorting.h"
#include "searching.h"
#include "history.h"

#define MENU_USER_MAX   10
#define MENU_ADMIN_MAX  13

/* =====================================================================
 *  BAGIAN 1: INISIALISASI DAN PENUTUPAN
 * ===================================================================== */

/*
 * Membangun BST dari data yang SUDAH TERURUT dengan selalu memasukkan
 * elemen tengah lebih dulu. Hasilnya pohon seimbang. Jika data terurut
 * dimasukkan berurutan, BST akan miring seperti linked list.
 */
static BSTNode *buildBalancedBST(BSTNode *root, const ParkingSlot *sorted, int low, int high,
                                 const ParkingLot *lot)
{
    int mid, index, steps = 0;

    if (low > high) {
        return root;
    }
    mid = low + (high - low) / 2;
    index = sequentialSearchById(lot->slots, lot->count, sorted[mid].id, &steps);
    root = bstInsert(root, sorted[mid].id, index);
    root = buildBalancedBST(root, sorted, low, mid - 1, lot);
    root = buildBalancedBST(root, sorted, mid + 1, high, lot);
    return root;
}

/* Membangun Hash Table (plat) dan BST (ID slot) dari array parkir */
static void buildIndexes(AppState *app)
{
    ParkingSlot *sorted;
    SortStats stats;
    int i;

    initHashTable(&app->plateTable);
    for (i = 0; i < app->lot.count; i++) {
        if (app->lot.slots[i].isOccupied) {
            hashInsert(&app->plateTable, app->lot.slots[i].plate, i);
        }
    }

    app->slotIndex = NULL;
    sorted = copySlotArray(&app->lot);
    if (sorted != NULL) {
        selectionSort(sorted, app->lot.count, SORT_BY_ID, ORDER_ASC, &stats);
        app->slotIndex = buildBalancedBST(NULL, sorted, 0, app->lot.count - 1, &app->lot);
        free(sorted);
    } else {
        for (i = 0; i < app->lot.count; i++) {   /* cadangan jika malloc gagal */
            app->slotIndex = bstInsert(app->slotIndex, app->lot.slots[i].id, i);
        }
    }
}

int initApp(AppState *app)
{
    char message[LINE_LEN];
    int needPause = 0;
    int skipped = 0;

    FILE *probe;

    app->isActive = 0;
    app->slotIndex = NULL;

    /* Cek folder data/ bisa ditulis. Gagal berarti program dijalankan
       dari folder yang salah (misalnya dari dalam folder src/). */
    probe = fopen(FILE_USERS, "a");
    if (probe == NULL) {
        printError("Folder data/ tidak ditemukan. Jalankan program dari folder utama proyek.");
        return 0;
    }
    fclose(probe);

    loadUsers(&app->users);

    if (!initParkingLot(&app->lot, MAX_SLOT)) {
        printError("Memori tidak cukup untuk data parkir.");
        return 0;
    }
    if (!loadParking(&app->lot, &skipped)) {
        printWarning("parking.txt tidak ditemukan atau kosong. Membuat 48 slot bawaan.");
        createDefaultSlots(&app->lot);
        saveParking(&app->lot);
        needPause = 1;
    }
    if (skipped > 0) {
        snprintf(message, sizeof(message),
                 "%d baris parking.txt tidak valid dan dilewati.", skipped);
        printWarning(message);
        needPause = 1;
    }

    initQueue(&app->carQueue);
    loadQueue(&app->carQueue);
    initStack(&app->activityStack);
    buildIndexes(app);

    initGraph(&app->mallMap);
    if (!loadGraph(&app->mallMap)) {
        printWarning("graph.txt tidak ditemukan atau tidak valid. Membuat denah bawaan.");
        createDefaultGraph(&app->mallMap);
        saveGraph(&app->mallMap);
        needPause = 1;
    }

    app->isActive = 1;
    if (needPause) {
        pauseScreen();
    }
    return 1;
}

/* Menyimpan data lalu membebaskan SEMUA memori hasil malloc */
void shutdownApp(AppState *app)
{
    if (!app->isActive) {
        return;     /* sudah pernah dibersihkan */
    }
    saveParking(&app->lot);
    saveQueue(&app->carQueue);

    freeHashTable(&app->plateTable);
    bstFree(app->slotIndex);
    app->slotIndex = NULL;
    freeStack(&app->activityStack);
    freeQueue(&app->carQueue);
    freeParkingLot(&app->lot);
    app->isActive = 0;
}

/* =====================================================================
 *  BAGIAN 2: FUNGSI BANTU
 * ===================================================================== */

static int gateNode(const AppState *app)
{
    int gate = findNodeByName(&app->mallMap, GATE_NAME);
    return (gate == -1) ? 0 : gate;
}

/* Membaca plat, menormalkan, dan memvalidasi. Mengembalikan 1 jika valid. */
static int readPlate(const char *prompt, char *plate)
{
    char raw[LINE_LEN];

    if (readString(prompt, raw, LINE_LEN) == 0) {
        printError("Nomor polisi tidak boleh kosong.");
        return 0;
    }
    if (!normalizePlate(raw, plate) || !isValidPlate(plate)) {
        printError("Format nomor polisi tidak valid. Contoh yang benar: H 1234 AB");
        return 0;
    }
    return 1;
}

/* Membaca ID slot dan mengubahnya ke huruf besar */
static int readSlotId(char *slotId)
{
    char raw[LINE_LEN];

    if (readString("  ID slot (contoh: B1-A02) : ", raw, LINE_LEN) == 0) {
        printError("ID slot tidak boleh kosong.");
        return 0;
    }
    toUpperCase(raw);
    if (strlen(raw) >= ID_LEN) {
        printError("ID slot terlalu panjang.");
        return 0;
    }
    strcpy(slotId, raw);
    return 1;
}

/* Mencari indeks slot lewat BST. -1 jika tidak ada. */
static int findSlotIndex(AppState *app, const char *slotId)
{
    BSTNode *node = bstSearch(app->slotIndex, slotId, NULL);
    return (node == NULL) ? -1 : node->slotIndex;
}

/* Mengisi struktur Activity sebelum status slot diubah */
static void prepareActivity(Activity *activity, int action, const ParkingSlot *slot,
                            const char *plate, const char *username)
{
    activity->action = action;
    snprintf(activity->slotId, ID_LEN, "%s", slot->id);
    snprintf(activity->plate, PLATE_LEN, "%s", plate);
    getCurrentTime(activity->time, TIME_LEN);
    snprintf(activity->username, NAME_LEN, "%s", username);
    activity->prevOccupied = slot->isOccupied;
    snprintf(activity->prevPlate, PLATE_LEN, "%s", slot->plate);
    snprintf(activity->prevEntryTime, TIME_LEN, "%s", slot->entryTime);
}

/* Menampilkan rute BFS dari pintu masuk menuju blok tempat slot berada */
static void showRouteToSlot(AppState *app, const ParkingSlot *slot)
{
    int path[MAX_NODE];
    int length = 0;
    int target = findBlockNode(&app->mallMap, slot->floor, slot->block);

    if (target == -1) {
        printError("Blok slot ini tidak ada pada denah.");
        return;
    }
    if (!findRouteBFS(&app->mallMap, gateNode(app), target, path, &length)) {
        printError("Tidak ada jalur dari pintu masuk menuju blok tersebut.");
        return;
    }
    printRoute(&app->mallMap, path, length, slot->id);
}

/*
 * Mencari slot kosong terdekat: BFS menentukan blok terdekat, lalu dipilih
 * slot kosong bernomor terkecil di blok itu. Mengembalikan indeks slot atau -1.
 */
static int findNearestEmptySlot(AppState *app, int *path, int *length, int showSteps)
{
    int target = findNearestEmptyBlock(&app->mallMap, &app->lot, gateNode(app),
                                       path, length, showSteps);
    const GraphNode *block;

    if (target == -1) {
        return -1;
    }
    block = &app->mallMap.nodes[target];
    return findFirstEmptyInBlock(&app->lot, block->floor, block->block);
}

/* =====================================================================
 *  BAGIAN 3: DASHBOARD
 * ===================================================================== */

static void showDashboard(const AppState *app)
{
    int total = app->lot.count;
    int empty = countEmptySlots(&app->lot);
    double percent = (total > 0) ? (empty * 100.0 / total) : 0.0;
    const char *floor;
    int i;

    printBoxTitle("DASHBOARD PARKIR");
    printf("  Pengguna aktif : %s%s%s (%s)\n", clr(CLR_CYAN), app->currentUser.username,
           clr(CLR_RESET), roleToText(app->currentUser.role));
    printf("  Total Slot     : %d\n", total);
    printf("  Slot Kosong    : %s%d%s\n", clr(CLR_GREEN), empty, clr(CLR_RESET));
    printf("  Slot Terisi    : %s%d%s\n", clr(CLR_RED), total - empty, clr(CLR_RESET));
    printf("  Antrean Mobil  : %s%d%s\n", clr(CLR_YELLOW), app->carQueue.size, clr(CLR_RESET));
    printf("  Ketersediaan   : ");
    printProgressBar(empty, total, 30);
    printf(" %.1f%%\n", percent);
    printf("  Kosong/lantai  :");
    for (i = FLOOR_COUNT - 1; i >= 0; i--) {
        floor = getFloorName(i);
        printf("  %s%s%s %d/%d", clr(CLR_CYAN), floor, clr(CLR_RESET),
               countEmptyOnFloor(&app->lot, floor), countSlotsOnFloor(&app->lot, floor));
    }
    printf("\n");
    if (total > 0 && empty == 0) {
        printWarning("PARKIR PENUH. Kendaraan baru akan menunggu di antrean.");
    }
}

/* =====================================================================
 *  BAGIAN 4: ANTREAN, MASUK, DAN KELUAR
 * ===================================================================== */

/* Proses mobil terdepan: dequeue -> BFS -> isi slot -> hash -> stack -> file */
static void processNextVehicle(AppState *app)
{
    int path[MAX_NODE];
    int length = 0;
    int slotIndex;
    char plate[PLATE_LEN];
    char now[TIME_LEN];
    char message[LINE_LEN];
    ParkingSlot *slot;
    Activity activity;

    if (isQueueEmpty(&app->carQueue)) {
        printWarning("Antrean kosong. Tidak ada kendaraan yang perlu diproses.");
        return;
    }
    if (countEmptySlots(&app->lot) == 0) {
        snprintf(message, sizeof(message), "Parkir penuh. %s tetap menunggu di depan antrean.",
                 peekQueue(&app->carQueue)->plate);
        printWarning(message);
        return;
    }

    printf("\n  Mobil terdepan : %s%s%s\n", clr(CLR_YELLOW), peekQueue(&app->carQueue)->plate,
           clr(CLR_RESET));
    slotIndex = findNearestEmptySlot(app, path, &length, 1);
    if (slotIndex == -1) {
        printError("Slot kosong ada, tetapi tidak terjangkau dari pintu masuk.");
        return;
    }

    dequeue(&app->carQueue, plate, NULL);
    slot = &app->lot.slots[slotIndex];
    prepareActivity(&activity, ACT_PARK, slot, plate, app->currentUser.username);

    getCurrentTime(now, TIME_LEN);
    occupySlot(slot, plate, now);
    hashInsert(&app->plateTable, plate, slotIndex);
    recordActivity(&app->activityStack, &activity);
    saveParking(&app->lot);
    saveQueue(&app->carQueue);

    snprintf(message, sizeof(message), "Kendaraan %s diarahkan ke slot %s.", plate, slot->id);
    printSuccess(message);
    printRoute(&app->mallMap, path, length, slot->id);
}

static void menuVehicleEntry(AppState *app)
{
    char plate[PLATE_LEN];
    char now[TIME_LEN];
    char message[LINE_LEN];
    int slotIndex, position;

    clearScreen();
    printBoxTitle("KENDARAAN MASUK");
    printInfo("Setiap kendaraan yang datang masuk antrean (Queue) lebih dulu.\n");

    if (!readPlate("  Nomor polisi (contoh: H 1234 AB) : ", plate)) {
        pauseScreen();
        return;
    }

    slotIndex = hashSearch(&app->plateTable, plate, NULL);
    if (slotIndex != -1) {
        snprintf(message, sizeof(message), "Kendaraan %s sudah terparkir di slot %s.",
                 plate, app->lot.slots[slotIndex].id);
        printError(message);
        pauseScreen();
        return;
    }
    position = findInQueue(&app->carQueue, plate);
    if (position > 0) {
        snprintf(message, sizeof(message), "Kendaraan %s sudah berada di antrean posisi %d.",
                 plate, position);
        printError(message);
        pauseScreen();
        return;
    }

    getCurrentTime(now, TIME_LEN);
    if (!enqueue(&app->carQueue, plate, now)) {
        printError("Memori tidak cukup untuk menambah antrean.");
        pauseScreen();
        return;
    }
    saveQueue(&app->carQueue);
    snprintf(message, sizeof(message), "Kendaraan %s masuk antrean pada posisi %d.",
             plate, app->carQueue.size);
    printSuccess(message);

    if (countEmptySlots(&app->lot) == 0) {
        printWarning("Parkir sedang penuh. Kendaraan menunggu sampai ada slot kosong.");
    } else if (readConfirm("  Proses antrean terdepan sekarang?")) {
        processNextVehicle(app);
    }
    pauseScreen();
}

static void menuProcessQueue(AppState *app)
{
    clearScreen();
    printBoxTitle("PROSES ANTREAN BERIKUTNYA");
    processNextVehicle(app);
    pauseScreen();
}

static void menuShowQueue(AppState *app)
{
    char message[LINE_LEN];

    clearScreen();
    printBoxTitle("DAFTAR ANTREAN KENDARAAN");
    displayQueue(&app->carQueue);
    snprintf(message, sizeof(message), "Jumlah antrean: %d mobil. Slot kosong tersedia: %d.",
             app->carQueue.size, countEmptySlots(&app->lot));
    printInfo(message);
    pauseScreen();
}

static void menuVehicleExit(AppState *app)
{
    char plate[PLATE_LEN];
    char message[LINE_LEN];
    int slotIndex, steps = 0, position;
    ParkingSlot *slot;
    Activity activity;

    clearScreen();
    printBoxTitle("KENDARAAN KELUAR");
    if (!readPlate("  Nomor polisi : ", plate)) {
        pauseScreen();
        return;
    }

    slotIndex = hashSearch(&app->plateTable, plate, &steps);   /* pencarian cepat lewat hash */
    if (slotIndex == -1) {
        printError("Kendaraan tidak ditemukan di area parkir.");
        position = findInQueue(&app->carQueue, plate);
        if (position > 0) {
            snprintf(message, sizeof(message), "Kendaraan ini masih di antrean posisi %d.", position);
            printInfo(message);
        }
        pauseScreen();
        return;
    }

    slot = &app->lot.slots[slotIndex];
    snprintf(message, sizeof(message), "Ditemukan lewat Hash Table (bucket %u, %d perbandingan).",
             hashFunction(plate), steps);
    printInfo(message);
    printSingleSlot(slot);

    if (!readConfirm("  Proses kendaraan keluar?")) {
        printInfo("Dibatalkan.");
        pauseScreen();
        return;
    }

    prepareActivity(&activity, ACT_LEAVE, slot, plate, app->currentUser.username);
    releaseSlot(slot);
    hashDelete(&app->plateTable, plate);
    recordActivity(&app->activityStack, &activity);
    saveParking(&app->lot);

    snprintf(message, sizeof(message), "Kendaraan %s keluar. Slot %s kini kosong.", plate, slot->id);
    printSuccess(message);
    if (!isQueueEmpty(&app->carQueue)) {
        snprintf(message, sizeof(message),
                 "%d mobil menunggu. Gunakan menu Proses Antrean Berikutnya.", app->carQueue.size);
        printInfo(message);
    }
    pauseScreen();
}

/* =====================================================================
 *  BAGIAN 5: PENCARIAN
 * ===================================================================== */

/* Sequential Search: cari slot kosong berdasarkan lantai dan blok */
static void searchEmptyByLocation(AppState *app)
{
    char floor[LINE_LEN];
    char blockText[LINE_LEN];
    char message[LINE_LEN];
    int results[MAX_SLOT];
    int found, steps = 0;
    char block;

    printf("\n");
    readString("  Lantai (B2/B1/P1/P2, ENTER = semua) : ", floor, LINE_LEN);
    toUpperCase(floor);
    if (floor[0] != '\0' && !isValidFloor(floor)) {
        printError("Lantai tidak valid. Pilih B2, B1, P1, atau P2.");
        return;
    }
    readString("  Blok   (A/B/C/D,   ENTER = semua) : ", blockText, LINE_LEN);
    toUpperCase(blockText);
    if (blockText[0] == '\0') {
        block = '*';
    } else if (strlen(blockText) == 1 && isValidBlock(blockText[0])) {
        block = blockText[0];
    } else {
        printError("Blok tidak valid. Pilih A, B, C, atau D.");
        return;
    }

    found = sequentialSearchEmpty(app->lot.slots, app->lot.count, floor, block,
                                  results, MAX_SLOT, &steps);
    snprintf(message, sizeof(message), "Sequential Search memeriksa %d data satu per satu.", steps);
    printInfo(message);

    if (found == 0) {
        if (countEmptySlots(&app->lot) == 0) {
            printError("Semua slot parkir penuh.");
        } else {
            printWarning("Tidak ada slot kosong di lokasi tersebut.");
        }
        return;
    }
    snprintf(message, sizeof(message), "Ditemukan %d slot kosong.", found);
    printSuccess(message);
    printSlotTableByIndex(&app->lot, results, found);

    if (readConfirm("  Tampilkan rute ke slot pertama?")) {
        showRouteToSlot(app, &app->lot.slots[results[0]]);
    }
}

/* Menampilkan hasil pencarian ID dan menawarkan rute jika slot kosong */
static void showSearchResult(AppState *app, const ParkingSlot *slot)
{
    char message[LINE_LEN];

    printSingleSlot(slot);
    if (slot->isOccupied) {
        snprintf(message, sizeof(message), "Slot %s sedang terisi oleh %s.", slot->id, slot->plate);
        printWarning(message);
    } else {
        printSuccess("Slot ini kosong dan bisa digunakan.");
        if (readConfirm("  Tampilkan rute ke slot ini?")) {
            showRouteToSlot(app, slot);
        }
    }
}

/* Binary Search: data disalin, diurutkan berdasarkan ID, lalu dicari */
static void searchSlotBinary(AppState *app)
{
    char slotId[ID_LEN];
    char message[LINE_LEN];
    ParkingSlot *sorted;
    SortStats stats;
    int position, binarySteps = 0, sequentialSteps = 0;

    printf("\n");
    if (!readSlotId(slotId)) {
        return;
    }
    sorted = copySlotArray(&app->lot);
    if (sorted == NULL) {
        printError("Data parkir kosong.");
        return;
    }

    /* Binary Search wajib pada data yang terurut berdasarkan kunci yang sama (ID) */
    bubbleSort(sorted, app->lot.count, SORT_BY_ID, ORDER_ASC, &stats);
    position = binarySearchById(sorted, app->lot.count, slotId, &binarySteps);
    sequentialSearchById(app->lot.slots, app->lot.count, slotId, &sequentialSteps);

    printInfo("Data disalin lalu diurutkan berdasarkan ID (Bubble Sort) sebelum Binary Search.");
    printf("  Binary Search     : %d perbandingan\n", binarySteps);
    printf("  Sequential Search : %d perbandingan (sebagai pembanding)\n", sequentialSteps);

    if (position == -1) {
        snprintf(message, sizeof(message), "Slot dengan ID %s tidak ditemukan.", slotId);
        printError(message);
    } else {
        showSearchResult(app, &sorted[position]);
    }
    free(sorted);
}

/* BST: pencarian ID lewat indeks pohon */
static void searchSlotBST(AppState *app)
{
    char slotId[ID_LEN];
    char message[LINE_LEN];
    BSTNode *node;
    int steps = 0;

    printf("\n");
    if (!readSlotId(slotId)) {
        return;
    }
    node = bstSearch(app->slotIndex, slotId, &steps);
    snprintf(message, sizeof(message), "BST mengunjungi %d node (tinggi pohon %d).",
             steps, bstHeight(app->slotIndex));
    printInfo(message);

    if (node == NULL) {
        snprintf(message, sizeof(message), "Slot dengan ID %s tidak ditemukan.", slotId);
        printError(message);
        return;
    }
    showSearchResult(app, &app->lot.slots[node->slotIndex]);
}

static void menuSearchSlot(AppState *app)
{
    int choice;

    do {
        clearScreen();
        printBoxTitle("CARI SLOT PARKIR");
        printMenuItem(1, "Slot kosong berdasarkan lokasi   (Sequential Search)");
        printMenuItem(2, "Slot berdasarkan ID              (Binary Search)");
        printMenuItem(3, "Slot berdasarkan ID              (Binary Search Tree)");
        printMenuItem(0, "Kembali");
        choice = readInt("\n  Pilih menu : ", 0, 3);

        switch (choice) {
            case 1: searchEmptyByLocation(app); break;
            case 2: searchSlotBinary(app); break;
            case 3: searchSlotBST(app); break;
            default: break;
        }
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

/* Hash Table: mencari lokasi mobil berdasarkan nomor polisi */
static void menuFindMyCar(AppState *app)
{
    char plate[PLATE_LEN];
    char message[LINE_LEN];
    int slotIndex, steps = 0, position;

    clearScreen();
    printBoxTitle("CARI MOBIL SAYA");
    printInfo("Lupa parkir di mana? Masukkan nomor polisi kendaraan Anda.\n");
    if (!readPlate("  Nomor polisi : ", plate)) {
        pauseScreen();
        return;
    }

    slotIndex = hashSearch(&app->plateTable, plate, &steps);
    snprintf(message, sizeof(message), "Hash(%s) = bucket %u, %d perbandingan di rantai.",
             plate, hashFunction(plate), steps);
    printInfo(message);

    if (slotIndex == -1) {
        printError("Kendaraan tidak ditemukan di area parkir.");
        position = findInQueue(&app->carQueue, plate);
        if (position > 0) {
            snprintf(message, sizeof(message), "Kendaraan ini masih di antrean posisi %d.", position);
            printInfo(message);
        }
    } else {
        printSuccess("Kendaraan ditemukan.");
        printSingleSlot(&app->lot.slots[slotIndex]);
        showRouteToSlot(app, &app->lot.slots[slotIndex]);
    }
    pauseScreen();
}

/* =====================================================================
 *  BAGIAN 6: RUTE, DENAH, DATA, DAN PENGURUTAN
 * ===================================================================== */

static void routeToNearestEmpty(AppState *app)
{
    int path[MAX_NODE];
    int length = 0;
    int slotIndex;

    printf("\n");
    if (countEmptySlots(&app->lot) == 0) {
        printError("Semua slot penuh. Tidak ada rute yang bisa diberikan.");
        return;
    }
    slotIndex = findNearestEmptySlot(app, path, &length, 1);
    if (slotIndex == -1) {
        printError("Slot kosong tidak terjangkau dari pintu masuk.");
        return;
    }
    printRoute(&app->mallMap, path, length, app->lot.slots[slotIndex].id);
}

static void routeToSpecificSlot(AppState *app)
{
    char slotId[ID_LEN];
    char message[LINE_LEN];
    int slotIndex;

    printf("\n");
    if (!readSlotId(slotId)) {
        return;
    }
    slotIndex = findSlotIndex(app, slotId);
    if (slotIndex == -1) {
        snprintf(message, sizeof(message), "Slot dengan ID %s tidak ditemukan.", slotId);
        printError(message);
        return;
    }
    if (app->lot.slots[slotIndex].isOccupied) {
        printError("Slot tersebut sedang terisi. Rute hanya diberikan menuju slot kosong.");
        return;
    }
    showRouteToSlot(app, &app->lot.slots[slotIndex]);
}

static void menuRouteMap(AppState *app)
{
    int choice;

    do {
        clearScreen();
        printBoxTitle("RUTE & DENAH PARKIR");
        printMenuItem(1, "Rute ke slot kosong terdekat     (BFS)");
        printMenuItem(2, "Rute ke slot kosong tertentu     (BFS)");
        printMenuItem(3, "Telusuri seluruh area parkir     (DFS)");
        printMenuItem(4, "Lihat denah (daftar titik dan jalan)");
        printMenuItem(0, "Kembali");
        choice = readInt("\n  Pilih menu : ", 0, 4);

        switch (choice) {
            case 1: routeToNearestEmpty(app); break;
            case 2: routeToSpecificSlot(app); break;
            case 3:
                printf("\n");
                dfsExplore(&app->mallMap, &app->lot, gateNode(app));
                break;
            case 4:
                printf("\n");
                displayGraph(&app->mallMap, &app->lot);
                printFloorMap(&app->lot);
                break;
            default: break;
        }
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

static void sortParkingData(AppState *app)
{
    int algorithm, key, order;
    ParkingSlot *copy;
    SortStats stats;

    if (app->lot.count == 0) {
        printError("Data parkir kosong.");
        return;
    }
    printf("\n  Algoritma : [1] Bubble Sort   [2] Selection Sort\n");
    algorithm = readInt("  Pilih     : ", 1, 2);
    printf("  Kunci     : [1] ID Slot   [2] Lantai   [3] Nomor Slot\n");
    key = readInt("  Pilih     : ", SORT_BY_ID, SORT_BY_NUMBER);
    printf("  Urutan    : [1] Ascending   [2] Descending\n");
    order = readInt("  Pilih     : ", ORDER_ASC, ORDER_DESC);

    copy = copySlotArray(&app->lot);
    if (copy == NULL) {
        printError("Memori tidak cukup untuk menyalin data.");
        return;
    }
    if (algorithm == 1) {
        bubbleSort(copy, app->lot.count, key, order, &stats);
    } else {
        selectionSort(copy, app->lot.count, key, order, &stats);
    }

    printf("\n");
    printSection("HASIL PENGURUTAN");
    printf("  Algoritma: %s%s%s | Kunci: %s | Urutan: %s\n", clr(CLR_CYAN),
           (algorithm == 1) ? "Bubble Sort" : "Selection Sort", clr(CLR_RESET),
           sortKeyText(key), sortOrderText(order));
    printSlotTable(copy, app->lot.count);
    printf("  Perbandingan: %ld   Pertukaran: %ld   Jumlah data: %d\n",
           stats.comparisons, stats.swaps, app->lot.count);
    printInfo("Yang diurutkan adalah salinan data. Isi parking.txt tidak berubah.");
    free(copy);
}

static void menuDataSort(AppState *app)
{
    int choice;

    do {
        clearScreen();
        printBoxTitle("DATA & PENGURUTAN PARKIR");
        printMenuItem(1, "Tampilkan seluruh data slot");
        printMenuItem(2, "Peta status per lantai");
        printMenuItem(3, "Urutkan data (Bubble / Selection Sort)");
        printMenuItem(0, "Kembali");
        choice = readInt("\n  Pilih menu : ", 0, 3);

        switch (choice) {
            case 1:
                printf("\n");
                printSlotTable(app->lot.slots, app->lot.count);
                break;
            case 2: printFloorMap(&app->lot); break;
            case 3: sortParkingData(app); break;
            default: break;
        }
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

/* =====================================================================
 *  BAGIAN 7: RIWAYAT DAN PENGATURAN
 * ===================================================================== */

static void menuHistory(AppState *app)
{
    int choice;

    do {
        clearScreen();
        printBoxTitle("RIWAYAT PARKIR");
        printMenuItem(1, "Aktivitas terbaru sesi ini   (Stack)");
        printMenuItem(2, "Riwayat lengkap              (history.txt)");
        printMenuItem(0, "Kembali");
        choice = readInt("\n  Pilih menu : ", 0, 2);

        printf("\n");
        if (choice == 1) displaySessionHistory(&app->activityStack);
        if (choice == 2) displayHistoryFile();
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

static void menuSettings(void)
{
    int choice;

    clearScreen();
    printBoxTitle("PENGATURAN TAMPILAN");
    printf("  Status warna saat ini : %s\n\n", isColorEnabled() ? "AKTIF" : "NONAKTIF");
    printMenuItem(1, "Aktifkan warna");
    printMenuItem(2, "Nonaktifkan warna (untuk terminal yang tidak mendukung)");
    printMenuItem(0, "Kembali");
    choice = readInt("\n  Pilih menu : ", 0, 2);

    if (choice == 1) {
        setColorEnabled(1);
        printSuccess("Warna diaktifkan.");
    } else if (choice == 2) {
        setColorEnabled(0);
        printSuccess("Warna dinonaktifkan.");
    }
    if (choice != 0) {
        pauseScreen();
    }
}

/* =====================================================================
 *  BAGIAN 8: MENU ADMIN
 * ===================================================================== */

static void adminAddSlot(AppState *app)
{
    char floor[LINE_LEN];
    char blockText[LINE_LEN];
    char slotId[ID_LEN];
    char message[LINE_LEN];
    int number, index;

    printf("\n");
    readString("  Lantai (B2/B1/P1/P2) : ", floor, LINE_LEN);
    toUpperCase(floor);
    if (!isValidFloor(floor)) {
        printError("Lantai tidak valid. Pilih B2, B1, P1, atau P2.");
        return;
    }
    readString("  Blok   (A/B/C/D)     : ", blockText, LINE_LEN);
    toUpperCase(blockText);
    if (strlen(blockText) != 1 || !isValidBlock(blockText[0])) {
        printError("Blok tidak valid. Pilih A, B, C, atau D.");
        return;
    }
    number = readInt("  Nomor slot (1-99)    : ", 1, MAX_SLOT_NUMBER);
    makeSlotId(slotId, floor, blockText[0], number);

    if (bstSearch(app->slotIndex, slotId, NULL) != NULL) {     /* BST: cek ID unik */
        snprintf(message, sizeof(message), "Slot %s sudah ada.", slotId);
        printError(message);
        return;
    }
    if (app->lot.count >= app->lot.capacity) {
        printError("Kapasitas data slot sudah penuh.");
        return;
    }
    snprintf(message, sizeof(message), "  Tambahkan slot %s?", slotId);
    if (!readConfirm(message)) {
        printInfo("Dibatalkan.");
        return;
    }

    index = addSlot(&app->lot, floor, blockText[0], number);
    app->slotIndex = bstInsert(app->slotIndex, slotId, index);
    saveParking(&app->lot);
    appendHistoryFile("TAMBAH-SLOT", slotId, EMPTY_FIELD, app->currentUser.username);
    snprintf(message, sizeof(message), "Slot %s berhasil ditambahkan.", slotId);
    printSuccess(message);
}

static void adminChangeStatus(AppState *app)
{
    char slotId[ID_LEN];
    char plate[PLATE_LEN];
    char now[TIME_LEN];
    char message[LINE_LEN];
    int slotIndex, otherIndex;
    ParkingSlot *slot;
    Activity activity;

    printf("\n");
    if (!readSlotId(slotId)) {
        return;
    }
    slotIndex = findSlotIndex(app, slotId);
    if (slotIndex == -1) {
        snprintf(message, sizeof(message), "Slot dengan ID %s tidak ditemukan.", slotId);
        printError(message);
        return;
    }
    slot = &app->lot.slots[slotIndex];
    printSingleSlot(slot);

    if (slot->isOccupied) {
        snprintf(message, sizeof(message), "  Ubah %s menjadi KOSONG (kendaraan %s dikeluarkan)?",
                 slot->id, slot->plate);
        if (!readConfirm(message)) {
            printInfo("Dibatalkan.");
            return;
        }
        prepareActivity(&activity, ACT_SET_EMPTY, slot, slot->plate, app->currentUser.username);
        hashDelete(&app->plateTable, slot->plate);
        releaseSlot(slot);
    } else {
        if (!readPlate("  Nomor polisi kendaraan : ", plate)) {
            return;
        }
        otherIndex = hashSearch(&app->plateTable, plate, NULL);
        if (otherIndex != -1) {
            snprintf(message, sizeof(message), "Kendaraan %s sudah terparkir di slot %s.",
                     plate, app->lot.slots[otherIndex].id);
            printError(message);
            return;
        }
        if (findInQueue(&app->carQueue, plate) > 0) {
            printError("Kendaraan ini ada di antrean. Proses lewat menu antrean.");
            return;
        }
        snprintf(message, sizeof(message), "  Ubah %s menjadi TERISI oleh %s?", slot->id, plate);
        if (!readConfirm(message)) {
            printInfo("Dibatalkan.");
            return;
        }
        prepareActivity(&activity, ACT_SET_OCCUPIED, slot, plate, app->currentUser.username);
        getCurrentTime(now, TIME_LEN);
        occupySlot(slot, plate, now);
        hashInsert(&app->plateTable, plate, slotIndex);
    }

    recordActivity(&app->activityStack, &activity);
    saveParking(&app->lot);
    printSuccess("Status slot berhasil diubah. Perubahan ini bisa di-undo.");
}

static void adminDeleteSlot(AppState *app)
{
    char slotId[ID_LEN];
    char message[LINE_LEN];
    int slotIndex;

    printf("\n");
    if (!readSlotId(slotId)) {
        return;
    }
    slotIndex = findSlotIndex(app, slotId);
    if (slotIndex == -1) {
        snprintf(message, sizeof(message), "Slot dengan ID %s tidak ditemukan.", slotId);
        printError(message);
        return;
    }
    printSingleSlot(&app->lot.slots[slotIndex]);
    if (app->lot.slots[slotIndex].isOccupied) {
        printError("Slot masih terisi. Kosongkan slot terlebih dahulu sebelum dihapus.");
        return;
    }
    snprintf(message, sizeof(message), "  Hapus slot %s secara permanen?", slotId);
    if (!readConfirm(message)) {
        printInfo("Dibatalkan.");
        return;
    }

    /* Urutan penting: hapus dari BST, geser indeks, baru hapus dari array */
    app->slotIndex = bstDelete(app->slotIndex, slotId);
    bstShiftIndexes(app->slotIndex, slotIndex);
    hashShiftIndexes(&app->plateTable, slotIndex);
    deleteSlotAt(&app->lot, slotIndex);
    saveParking(&app->lot);
    appendHistoryFile("HAPUS-SLOT", slotId, EMPTY_FIELD, app->currentUser.username);

    snprintf(message, sizeof(message), "Slot %s berhasil dihapus.", slotId);
    printSuccess(message);
}

static void menuManageSlots(AppState *app)
{
    int choice;

    do {
        clearScreen();
        printBoxTitle("KELOLA SLOT PARKIR (ADMIN)");
        printMenuItem(1, "Tambah slot parkir");
        printMenuItem(2, "Ubah status slot (kosong / terisi)");
        printMenuItem(3, "Hapus slot parkir");
        printMenuItem(4, "Tampilkan seluruh data slot");
        printMenuItem(0, "Kembali");
        choice = readInt("\n  Pilih menu : ", 0, 4);

        switch (choice) {
            case 1: adminAddSlot(app); break;
            case 2: adminChangeStatus(app); break;
            case 3: adminDeleteSlot(app); break;
            case 4:
                printf("\n");
                printSlotTable(app->lot.slots, app->lot.count);
                break;
            default: break;
        }
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

/*
 * Undo: ambil aktivitas teratas Stack (LIFO), lalu kembalikan slot ke
 * kondisi sebelum aktivitas itu terjadi (prevOccupied, prevPlate, prevEntryTime).
 */
static void menuUndo(AppState *app)
{
    const Activity *top;
    Activity activity;
    ParkingSlot *slot;
    char message[LINE_LEN];
    int slotIndex, otherIndex;

    clearScreen();
    printBoxTitle("UNDO PERUBAHAN TERAKHIR");

    top = peekStack(&app->activityStack);
    if (top == NULL) {
        printWarning("Tidak ada aktivitas pada sesi ini yang bisa dibatalkan.");
        pauseScreen();
        return;
    }

    printf("  Aktivitas teratas Stack:\n");
    printf("  Aksi : %s%s%s\n", clr(CLR_YELLOW), actionToText(top->action), clr(CLR_RESET));
    printf("  Slot : %s\n  Plat : %s\n  Oleh : %s pada %s\n", top->slotId, top->plate,
           top->username, top->time);
    printf("  Akan dikembalikan ke: %s", top->prevOccupied ? "TERISI oleh " : "KOSONG");
    printf("%s\n\n", top->prevOccupied ? top->prevPlate : "");

    slotIndex = findSlotIndex(app, top->slotId);
    if (slotIndex == -1) {
        printError("Slot sudah dihapus, aktivitas ini tidak bisa di-undo.");
        pop(&app->activityStack, NULL);
        pauseScreen();
        return;
    }
    if (top->prevOccupied) {
        otherIndex = hashSearch(&app->plateTable, top->prevPlate, NULL);
        if (otherIndex != -1 && otherIndex != slotIndex) {
            printError("Kendaraan tersebut sekarang terparkir di slot lain. Undo dibatalkan.");
            pauseScreen();
            return;
        }
    }
    if (!readConfirm("  Batalkan aktivitas ini?")) {
        printInfo("Dibatalkan.");
        pauseScreen();
        return;
    }

    pop(&app->activityStack, &activity);
    slot = &app->lot.slots[slotIndex];

    if (slot->isOccupied) {
        hashDelete(&app->plateTable, slot->plate);
    }
    if (activity.prevOccupied) {
        occupySlot(slot, activity.prevPlate, activity.prevEntryTime);
        hashInsert(&app->plateTable, activity.prevPlate, slotIndex);
    } else {
        releaseSlot(slot);
    }
    saveParking(&app->lot);

    snprintf(message, sizeof(message), "UNDO-%s", actionToText(activity.action));
    appendHistoryFile(message, activity.slotId, activity.plate, app->currentUser.username);
    snprintf(message, sizeof(message), "Undo berhasil. Slot %s kembali %s.", activity.slotId,
             activity.prevOccupied ? "TERISI" : "KOSONG");
    printSuccess(message);
    if (activity.action == ACT_PARK) {
        printInfo("Catatan: kendaraan tidak dikembalikan ke antrean.");
    }
    pauseScreen();
}

static void menuIndexes(AppState *app)
{
    int choice, counter;

    do {
        clearScreen();
        printBoxTitle("INDEKS DATA (ADMIN)");
        printMenuItem(1, "Binary Search Tree ID slot (traversal)");
        printMenuItem(2, "Hash Table nomor polisi (bucket dan collision)");
        printMenuItem(0, "Kembali");
        choice = readInt("\n  Pilih menu : ", 0, 2);

        if (choice == 1) {
            printf("\n  Jumlah node : %d   Tinggi pohon : %d   Root : %s\n",
                   bstCountNodes(app->slotIndex), bstHeight(app->slotIndex),
                   app->slotIndex ? app->slotIndex->slotId : "-");
            counter = 0;
            printf("\n  %sPre-order  (node, kiri, kanan):%s", clr(CLR_MAGENTA), clr(CLR_RESET));
            bstPreOrder(app->slotIndex, &counter);
            counter = 0;
            printf("\n\n  %sIn-order   (kiri, node, kanan) = ID terurut:%s", clr(CLR_MAGENTA), clr(CLR_RESET));
            bstInOrder(app->slotIndex, &counter);
            counter = 0;
            printf("\n\n  %sPost-order (kiri, kanan, node):%s", clr(CLR_MAGENTA), clr(CLR_RESET));
            bstPostOrder(app->slotIndex, &counter);
            printf("\n");
        } else if (choice == 2) {
            printf("\n");
            displayHashTable(&app->plateTable, &app->lot);
        }
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

/* =====================================================================
 *  BAGIAN 9: MENU UTAMA DAN MENU AWAL
 * ===================================================================== */

static void printMainMenu(int isAdmin)
{
    printSection("MENU UTAMA");
    printMenuItem(1, "Kendaraan Masuk (antre)");
    printMenuItem(2, "Proses Antrean Berikutnya");
    printMenuItem(3, "Lihat Antrean Kendaraan");
    printMenuItem(4, "Kendaraan Keluar");
    printMenuItem(5, "Cari Slot Parkir");
    printMenuItem(6, "Cari Mobil Saya");
    printMenuItem(7, "Rute & Denah Parkir");
    printMenuItem(8, "Data & Pengurutan Parkir");
    printMenuItem(9, "Riwayat Parkir");
    printMenuItem(10, "Pengaturan Tampilan");
    if (isAdmin) {
        printAdminMenuItem(11, "Kelola Slot Parkir");
        printAdminMenuItem(12, "Undo Perubahan Terakhir");
        printAdminMenuItem(13, "Indeks Data (BST & Hash Table)");
    }
    printMenuItem(0, "Logout");
    printLine('-');
}

static void runMainMenu(AppState *app)
{
    int isAdmin = (app->currentUser.role == ROLE_ADMIN);
    int running = 1;
    int choice;

    while (running) {
        clearScreen();
        printHeader();
        showDashboard(app);
        printMainMenu(isAdmin);
        choice = readInt("  Pilih menu : ", 0, isAdmin ? MENU_ADMIN_MAX : MENU_USER_MAX);

        switch (choice) {
            case 1:  menuVehicleEntry(app); break;
            case 2:  menuProcessQueue(app); break;
            case 3:  menuShowQueue(app); break;
            case 4:  menuVehicleExit(app); break;
            case 5:  menuSearchSlot(app); break;
            case 6:  menuFindMyCar(app); break;
            case 7:  menuRouteMap(app); break;
            case 8:  menuDataSort(app); break;
            case 9:  menuHistory(app); break;
            case 10: menuSettings(); break;
            case 11: menuManageSlots(app); break;
            case 12: menuUndo(app); break;
            case 13: menuIndexes(app); break;
            case 0:
                if (readConfirm("  Yakin ingin logout?")) {
                    running = 0;
                    printSuccess("Logout berhasil. Kembali ke menu awal.");
                    pauseScreen();
                }
                break;
            default: break;
        }
    }
}

void runStartMenu(AppState *app)
{
    int choice;

    do {
        clearScreen();
        printHeader();
        printBoxTitle("MENU AWAL");
        printMenuItem(1, "Login");
        printMenuItem(2, "Register");
        printMenuItem(0, "Keluar Aplikasi");
        printLine('-');
        choice = readInt("  Pilih menu : ", 0, 2);

        clearScreen();
        if (choice == 1) {
            if (loginUser(&app->users, &app->currentUser)) {
                pauseScreen();
                runMainMenu(app);
            } else {
                pauseScreen();
            }
        } else if (choice == 2) {
            registerUser(&app->users);
            pauseScreen();
        }
    } while (choice != 0);

    printSuccess("Data tersimpan. Terima kasih telah menggunakan SMART PARKING MALL.");
}
