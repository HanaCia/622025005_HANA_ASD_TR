/*
 * main.c
 * Titik awal program SMART PARKING MALL.
 * Tugas main hanya tiga: menyiapkan tampilan, memuat data, dan menjalankan
 * menu awal. Seluruh fitur berada di modul lain.
 * Perintah kompilasi ada di README.md.
 */
#include <stdlib.h>
#include "menu.h"
#include "display.h"

static AppState app;   /* global agar bisa diakses fungsi pembersih atexit */

/* Dipanggil otomatis saat program berakhir: simpan data dan free memori */
static void cleanUp(void)
{
    shutdownApp(&app);
}

int main(void)
{
    initDisplay();
    atexit(cleanUp);   /* didaftarkan lebih dulu agar memori selalu dibebaskan */
    if (!initApp(&app)) {
        printError("Aplikasi gagal dimulai.");
        return EXIT_FAILURE;
    }

    runStartMenu(&app);
    return EXIT_SUCCESS;
}
