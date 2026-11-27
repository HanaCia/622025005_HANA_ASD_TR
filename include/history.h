/*
 * history.h
 * Riwayat parkir: aktivitas sesi berjalan disimpan di Stack, dan semua
 * aktivitas juga ditulis permanen ke history.txt.
 */
#ifndef HISTORY_H
#define HISTORY_H

#include "stack.h"

const char *actionToText(int action);
int  appendHistoryFile(const char *actionText, const char *slotId,
                       const char *plate, const char *username);
int  recordActivity(Stack *stack, const Activity *activity);
void displaySessionHistory(const Stack *stack);
void displayHistoryFile(void);

#endif
