/*
 * input.c
 * Implementasi pembacaan input yang aman dan utilitas teks/waktu.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "input.h"
#include "display.h"
#include "config.h"

/*
 * Membaca satu baris dari keyboard ke buffer.
 * - Karakter '\n' di akhir dibuang.
 * - Jika input lebih panjang dari buffer, sisanya dibuang agar tidak
 *   "terbawa" ke input berikutnya.
 * - Jika input berakhir (EOF), program ditutup dengan rapi. Fungsi
 *   pembersih memori tetap dijalankan karena didaftarkan lewat atexit().
 */
int readLine(char *buffer, int size)
{
    int length;
    int ch;

    if (fgets(buffer, size, stdin) == NULL) {
        printf("\n");
        printWarning("Input berakhir. Program ditutup.");
        exit(0);
    }

    length = (int) strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* buang sisa karakter */
        }
    }

    trimText(buffer);
    return (int) strlen(buffer);
}

/* Membaca bilangan bulat dalam rentang [min, max]. Mengulang bila salah. */
int readInt(const char *prompt, int min, int max)
{
    char buffer[LINE_LEN];
    char message[LINE_LEN];
    char *end;
    long value;

    while (1) {
        printf("%s", prompt);
        readLine(buffer, LINE_LEN);

        if (buffer[0] == '\0') {
            printError("Input tidak boleh kosong.");
            continue;
        }

        value = strtol(buffer, &end, 10);   /* ubah teks menjadi angka */
        if (*end != '\0') {
            printError("Input tidak valid. Masukkan angka.");
            continue;
        }
        if (value < min || value > max) {
            snprintf(message, sizeof(message),
                     "Pilihan harus berada di antara %d dan %d.", min, max);
            printError(message);
            continue;
        }
        return (int) value;
    }
}

/* Membaca teks. Mengembalikan panjang teks (0 berarti kosong). */
int readString(const char *prompt, char *buffer, int size)
{
    printf("%s", prompt);
    return readLine(buffer, size);
}

/* Konfirmasi Y/N. Mengembalikan 1 untuk Y dan 0 untuk N. */
int readConfirm(const char *prompt)
{
    char buffer[LINE_LEN];

    while (1) {
        printf("%s %s(Y/N)%s: ", prompt, clr(CLR_YELLOW), clr(CLR_RESET));
        readLine(buffer, LINE_LEN);
        if (strcmp(buffer, "Y") == 0 || strcmp(buffer, "y") == 0) {
            return 1;
        }
        if (strcmp(buffer, "N") == 0 || strcmp(buffer, "n") == 0) {
            return 0;
        }
        printError("Jawab dengan Y atau N.");
    }
}

void pauseScreen(void)
{
    char buffer[LINE_LEN];
    printf("\n  %sTekan ENTER untuk melanjutkan...%s", clr(CLR_GRAY), clr(CLR_RESET));
    readLine(buffer, LINE_LEN);
}

/* Menghapus spasi, tab, dan '\r' di awal dan akhir teks */
void trimText(char *text)
{
    int start = 0;
    int end = (int) strlen(text) - 1;
    int i;

    while (text[start] != '\0' && isspace((unsigned char) text[start])) {
        start++;
    }
    while (end >= start && isspace((unsigned char) text[end])) {
        end--;
    }
    for (i = start; i <= end; i++) {
        text[i - start] = text[i];
    }
    text[end - start + 1] = '\0';
}

void toUpperCase(char *text)
{
    int i;
    for (i = 0; text[i] != '\0'; i++) {
        text[i] = (char) toupper((unsigned char) text[i]);
    }
}

void toLowerCase(char *text)
{
    int i;
    for (i = 0; text[i] != '\0'; i++) {
        text[i] = (char) tolower((unsigned char) text[i]);
    }
}

/* Mengisi buffer dengan waktu sekarang, format "YYYY-MM-DD HH:MM" */
void getCurrentTime(char *buffer, int size)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local == NULL || strftime(buffer, (size_t) size, "%Y-%m-%d %H:%M", local) == 0) {
        snprintf(buffer, (size_t) size, "%s", EMPTY_FIELD);
    }
}
