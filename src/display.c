/*
 * display.c
 * Mengatur tampilan terminal: warna, header ASCII art, kotak, dan pesan.
 * Jika terminal tidak mendukung warna, program tetap berjalan tanpa warna.
 */
#include <stdio.h>
#include <string.h>
#include "display.h"
#include "config.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN   /* hanya bagian windows.h yang diperlukan */
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

static int colorEnabled = 1;   /* 1 = warna aktif, 0 = tanpa warna */

/* Mengaktifkan dukungan warna ANSI. Di Windows perlu mode khusus. */
void initDisplay(void)
{
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (output == INVALID_HANDLE_VALUE || !GetConsoleMode(output, &mode) ||
        !SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        colorEnabled = 0;   /* fallback: terminal tidak mendukung warna */
    }
#endif
}

void setColorEnabled(int enabled)
{
    colorEnabled = enabled;
}

int isColorEnabled(void)
{
    return colorEnabled;
}

/* Mengembalikan kode warna bila warna aktif, atau string kosong bila tidak */
const char *clr(const char *code)
{
    return colorEnabled ? code : "";
}

void clearScreen(void)
{
    if (colorEnabled) {
        printf("\033[2J\033[H");
    } else {
        printf("\n\n");
    }
}

void printLine(char symbol)
{
    int i;
    printf("%s", clr(CLR_BLUE));
    for (i = 0; i < SCREEN_WIDTH; i++) {
        putchar(symbol);
    }
    printf("%s\n", clr(CLR_RESET));
}

/* Mencetak teks di tengah baris selebar SCREEN_WIDTH */
static void printCentered(const char *text, const char *color)
{
    int padding = (SCREEN_WIDTH - (int) strlen(text)) / 2;
    if (padding < 0) {
        padding = 0;
    }
    printf("%*s%s%s%s\n", padding, "", clr(color), text, clr(CLR_RESET));
}

void printHeader(void)
{
    /* ASCII art judul. Tanda '\' ditulis '\\' karena aturan string C. */
    const char *art[] = {
        "  ____  __  __    _    ____ _____   ____   _    ____  _  _____ _   _  ____ ",
        " / ___||  \\/  |  / \\  |  _ \\_   _| |  _ \\ / \\  |  _ \\| |/ /_ _| \\ | |/ ___|",
        " \\___ \\| |\\/| | / _ \\ | |_) || |   | |_) / _ \\ | |_) | ' / | ||  \\| | |  _ ",
        "  ___) | |  | |/ ___ \\|  _ < | |   |  __/ ___ \\|  _ <| . \\ | || |\\  | |_| |",
        " |____/|_|  |_/_/   \\_\\_| \\_\\|_|   |_| /_/   \\_\\_| \\_\\_|\\_\\___|_| \\_|\\____|"
    };
    int lineCount = (int) (sizeof(art) / sizeof(art[0]));
    int i;

    printLine('=');
    for (i = 0; i < lineCount; i++) {
        printCentered(art[i], CLR_CYAN);
    }
    printCentered("[P]  M  A  L  L  [P]", CLR_MAGENTA);
    printCentered(APP_SUBTITLE, CLR_YELLOW);
    printCentered(APP_AUTHOR, CLR_GRAY);
    printLine('=');
}

/* Kotak judul:  +----+ / |  JUDUL  | / +----+ */
void printBoxTitle(const char *title)
{
    int inner = SCREEN_WIDTH - 2;
    int left = (inner - (int) strlen(title)) / 2;
    int right = inner - (int) strlen(title) - left;
    int i;

    printf("%s+", clr(CLR_BLUE));
    for (i = 0; i < inner; i++) putchar('-');
    printf("+\n|%s", clr(CLR_RESET));
    printf("%*s%s%s%s%s%*s", left, "", clr(CLR_BOLD), clr(CLR_YELLOW),
           title, clr(CLR_RESET), right, "");
    printf("%s|\n+", clr(CLR_BLUE));
    for (i = 0; i < inner; i++) putchar('-');
    printf("+%s\n", clr(CLR_RESET));
}

/* Judul bagian kecil:  ---[ JUDUL ]--------- */
void printSection(const char *title)
{
    int used = (int) strlen(title) + 8;
    int i;

    printf("%s  --[ %s%s%s ]", clr(CLR_BLUE), clr(CLR_MAGENTA), title, clr(CLR_BLUE));
    for (i = used; i < SCREEN_WIDTH; i++) putchar('-');
    printf("%s\n", clr(CLR_RESET));
}

void printMenuItem(int number, const char *text)
{
    printf("   %s[%2d]%s %s\n", clr(CLR_CYAN), number, clr(CLR_RESET), text);
}

void printAdminMenuItem(int number, const char *text)
{
    printf("   %s[%2d]%s %s %s(admin)%s\n", clr(CLR_MAGENTA), number, clr(CLR_RESET),
           text, clr(CLR_GRAY), clr(CLR_RESET));
}

void printSuccess(const char *message)
{
    printf("  %s[ OK ]%s %s\n", clr(CLR_GREEN), clr(CLR_RESET), message);
}

void printError(const char *message)
{
    printf("  %s[GAGAL]%s %s\n", clr(CLR_RED), clr(CLR_RESET), message);
}

void printWarning(const char *message)
{
    printf("  %s[ !! ]%s %s\n", clr(CLR_YELLOW), clr(CLR_RESET), message);
}

void printInfo(const char *message)
{
    printf("  %s[INFO]%s %s\n", clr(CLR_CYAN), clr(CLR_RESET), message);
}

/* Hijau = KOSONG, Merah = TERISI. Lebar teks selalu 6 karakter. */
void printStatusText(int isOccupied)
{
    if (isOccupied) {
        printf("%s%-6s%s", clr(CLR_RED), "TERISI", clr(CLR_RESET));
    } else {
        printf("%s%-6s%s", clr(CLR_GREEN), "KOSONG", clr(CLR_RESET));
    }
}

/* Bar ketersediaan, contoh: [##########----------] */
void printProgressBar(int value, int total, int width)
{
    int filled = (total > 0) ? (value * width / total) : 0;
    const char *color = CLR_GREEN;
    int i;

    if (total > 0 && value * 100 / total < 20) {
        color = CLR_RED;
    } else if (total > 0 && value * 100 / total < 50) {
        color = CLR_YELLOW;
    }

    printf("[%s", clr(color));
    for (i = 0; i < width; i++) {
        putchar(i < filled ? '#' : '-');
    }
    printf("%s]", clr(CLR_RESET));
}
