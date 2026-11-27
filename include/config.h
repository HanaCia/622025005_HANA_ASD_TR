/*
 * config.h
 * Kumpulan konstanta bernama yang dipakai di seluruh program.
 * Semua angka penting dikumpulkan di sini agar tidak ada "magic number"
 * yang tersebar di dalam kode.
 */
#ifndef CONFIG_H
#define CONFIG_H

/* ---------- Identitas aplikasi ---------- */
#define APP_SUBTITLE      "SMART QUEUE & PARKING ROUTE SYSTEM"
#define APP_AUTHOR        "Hana | 622025005 | Tugas Rancang ASD 2026"

/* ---------- Ukuran buffer string (sudah termasuk '\0') ---------- */
#define ID_LEN            16   /* contoh: "B1-A02"            */
#define FLOOR_LEN         4    /* contoh: "B1"                */
#define PLATE_LEN         12   /* contoh: "H1234AB"           */
#define TIME_LEN          20   /* contoh: "2026-11-27 10:15"  */
#define NAME_LEN          32
#define PASS_LEN          32
#define LINE_LEN          256

/* ---------- Kapasitas data ---------- */
#define MAX_SLOT          100  /* kapasitas array slot parkir (malloc)  */
#define MAX_USER          100  /* kapasitas akun                        */
#define HISTORY_SHOW_MAX  30   /* jumlah baris riwayat file yang tampil */

/* ---------- Denah bawaan ---------- */
#define FLOOR_COUNT       4    /* B2, B1, P1, P2 */
#define BLOCK_COUNT       4    /* A, B, C, D     */
#define SLOT_PER_BLOCK    3    /* 4 x 4 x 3 = 48 slot awal */
#define MAX_SLOT_NUMBER   99

/* ---------- Aturan akun ---------- */
#define USERNAME_MIN      3
#define USERNAME_MAX      20
#define PASSWORD_MIN      6
#define PASSWORD_MAX      20
#define ROLE_ADMIN        1
#define ROLE_USER         2
#define DEFAULT_ADMIN_NAME "admin"
#define DEFAULT_ADMIN_PASS "admin123"

/* ---------- Aturan nomor polisi (contoh: H 1234 AB) ---------- */
#define PLATE_MAX_PREFIX  2    /* huruf depan maksimal    */
#define PLATE_MAX_DIGIT   4    /* angka maksimal          */
#define PLATE_MAX_SUFFIX  3    /* huruf belakang maksimal */

/* ---------- Lokasi file database (.txt) ---------- */
#define FILE_USERS        "data/users.txt"
#define FILE_PARKING      "data/parking.txt"
#define FILE_HISTORY      "data/history.txt"
#define FILE_QUEUE        "data/queue.txt"
#define FILE_GRAPH        "data/graph.txt"

/* Penanda isi kosong di dalam file, misalnya plat pada slot kosong */
#define EMPTY_FIELD       "-"

#endif
