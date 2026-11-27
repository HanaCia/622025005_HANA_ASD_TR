/*
 * history.c
 * Mencatat aktivitas ke Stack (untuk undo) dan ke history.txt (permanen).
 * Format history.txt per baris:  waktu|aksi|idSlot|plat|username
 */
#include <stdio.h>
#include <string.h>
#include "history.h"
#include "input.h"
#include "display.h"

const char *actionToText(int action)
{
    switch (action) {
        case ACT_PARK:         return "MASUK";
        case ACT_LEAVE:        return "KELUAR";
        case ACT_SET_EMPTY:    return "SET-KOSONG";
        case ACT_SET_OCCUPIED: return "SET-TERISI";
        default:               return "LAINNYA";
    }
}

/* Menambahkan satu baris ke akhir history.txt (mode "a" = append) */
int appendHistoryFile(const char *actionText, const char *slotId,
                      const char *plate, const char *username)
{
    FILE *file = fopen(FILE_HISTORY, "a");
    char now[TIME_LEN];

    if (file == NULL) {
        printError("Gagal menulis history.txt.");
        return 0;
    }
    getCurrentTime(now, TIME_LEN);
    fprintf(file, "%s|%s|%s|%s|%s\n", now, actionText, slotId, plate, username);
    fclose(file);
    return 1;
}

/* Aktivitas masuk ke Stack (agar bisa di-undo) dan ke file (permanen) */
int recordActivity(Stack *stack, const Activity *activity)
{
    if (!push(stack, activity)) {
        printWarning("Memori penuh, aktivitas tidak masuk ke stack undo.");
    }
    return appendHistoryFile(actionToText(activity->action), activity->slotId,
                             activity->plate, activity->username);
}

static void printHistoryBorder(void)
{
    printf("  %s+-----+------------------+-----------------+---------+------------+----------+%s\n",
           clr(CLR_BLUE), clr(CLR_RESET));
}

static void printHistoryHeader(void)
{
    printHistoryBorder();
    printf("  | %s%-3s%s | %s%-16s%s | %s%-15s%s | %s%-7s%s | %s%-10s%s | %s%-8s%s |\n",
           clr(CLR_YELLOW), "No", clr(CLR_RESET), clr(CLR_YELLOW), "Waktu", clr(CLR_RESET),
           clr(CLR_YELLOW), "Aksi", clr(CLR_RESET), clr(CLR_YELLOW), "Slot", clr(CLR_RESET),
           clr(CLR_YELLOW), "No Polisi", clr(CLR_RESET), clr(CLR_YELLOW), "User", clr(CLR_RESET));
    printHistoryBorder();
}

static void printHistoryRow(int number, const char *time, const char *action,
                            const char *slotId, const char *plate, const char *user)
{
    const char *color = CLR_RESET;
    if (strcmp(action, "MASUK") == 0) color = CLR_GREEN;
    else if (strcmp(action, "KELUAR") == 0) color = CLR_RED;
    else if (strncmp(action, "UNDO", 4) == 0) color = CLR_MAGENTA;

    printf("  | %3d | %-16s | %s%-15.15s%s | %-7s | %-10s | %-8.8s |\n", number, time,
           clr(color), action, clr(CLR_RESET), slotId, plate, user);
}

/* Isi Stack ditampilkan dari atas (paling baru) ke bawah (paling lama) */
void displaySessionHistory(const Stack *stack)
{
    const StackNode *current = stack->top;
    int number = 1;

    if (isStackEmpty(stack)) {
        printInfo("Belum ada aktivitas pada sesi ini.");
        return;
    }
    printInfo("Urutan dari atas Stack: nomor 1 adalah aktivitas paling baru.");
    printHistoryHeader();
    while (current != NULL) {
        printHistoryRow(number, current->data.time, actionToText(current->data.action),
                        current->data.slotId, current->data.plate, current->data.username);
        current = current->next;
        number++;
    }
    printHistoryBorder();
}

/* Menampilkan HISTORY_SHOW_MAX baris terakhir dari history.txt */
void displayHistoryFile(void)
{
    FILE *file = fopen(FILE_HISTORY, "r");
    char line[LINE_LEN];
    char time[TIME_LEN], action[NAME_LEN], slotId[ID_LEN], plate[PLATE_LEN], user[NAME_LEN];
    char message[LINE_LEN];
    int totalLines = 0;
    int lineNumber = 0;
    int skipUntil;

    if (file == NULL) {
        printInfo("Belum ada riwayat tersimpan di history.txt.");
        return;
    }

    /* Langkah 1: hitung jumlah baris */
    while (fgets(line, sizeof(line), file) != NULL) {
        trimText(line);
        if (line[0] != '\0') totalLines++;
    }
    if (totalLines == 0) {
        fclose(file);
        printInfo("Belum ada riwayat tersimpan di history.txt.");
        return;
    }

    /* Langkah 2: baca ulang dari awal, tampilkan hanya baris terakhir */
    rewind(file);
    skipUntil = totalLines - HISTORY_SHOW_MAX;
    snprintf(message, sizeof(message), "Total %d riwayat di file. Menampilkan maksimal %d terakhir.",
             totalLines, HISTORY_SHOW_MAX);
    printInfo(message);
    printHistoryHeader();
    while (fgets(line, sizeof(line), file) != NULL) {
        trimText(line);
        if (line[0] == '\0') continue;
        lineNumber++;
        if (lineNumber <= skipUntil) continue;
        if (sscanf(line, "%19[^|]|%31[^|]|%15[^|]|%11[^|]|%31[^\n]",
                   time, action, slotId, plate, user) == 5) {
            printHistoryRow(lineNumber, time, action, slotId, plate, user);
        }
    }
    printHistoryBorder();
    fclose(file);
}
