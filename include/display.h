/*
 * display.h
 * Fungsi tampilan: warna ANSI, header, kotak judul, menu, dan pesan.
 */
#ifndef DISPLAY_H
#define DISPLAY_H

/* Kode warna ANSI. Selalu dipakai lewat fungsi clr() agar bisa dimatikan. */
#define CLR_RESET    "\033[0m"
#define CLR_BOLD     "\033[1m"
#define CLR_RED      "\033[91m"
#define CLR_GREEN    "\033[92m"
#define CLR_YELLOW   "\033[93m"
#define CLR_BLUE     "\033[94m"
#define CLR_MAGENTA  "\033[95m"
#define CLR_CYAN     "\033[96m"
#define CLR_GRAY     "\033[90m"

#define SCREEN_WIDTH 78

void initDisplay(void);
void setColorEnabled(int enabled);
int  isColorEnabled(void);
const char *clr(const char *code);

void clearScreen(void);
void printLine(char symbol);
void printHeader(void);
void printBoxTitle(const char *title);
void printSection(const char *title);
void printMenuItem(int number, const char *text);
void printAdminMenuItem(int number, const char *text);

void printSuccess(const char *message);
void printError(const char *message);
void printWarning(const char *message);
void printInfo(const char *message);

void printStatusText(int isOccupied);
void printProgressBar(int value, int total, int width);

#endif
