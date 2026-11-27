/*
 * input.h
 * Fungsi bantu membaca input dengan aman (tanpa scanf) dan utilitas kecil.
 * Semua input dibaca per baris dengan fgets agar input salah seperti "abc"
 * tidak membuat program crash atau terjebak infinite loop.
 */
#ifndef INPUT_H
#define INPUT_H

int  readLine(char *buffer, int size);
int  readInt(const char *prompt, int min, int max);
int  readString(const char *prompt, char *buffer, int size);
int  readConfirm(const char *prompt);
void pauseScreen(void);

void trimText(char *text);
void toUpperCase(char *text);
void toLowerCase(char *text);
void getCurrentTime(char *buffer, int size);

#endif
