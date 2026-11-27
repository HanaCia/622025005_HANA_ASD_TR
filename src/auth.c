/*
 * auth.c
 * Register dan login akun. Data akun disimpan di data/users.txt dengan
 * format per baris:  username|password|role
 * Role 1 = ADMIN, role 2 = USER. Akun admin bawaan dibuat otomatis
 * bila belum ada admin di file.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "auth.h"
#include "input.h"
#include "display.h"

/* Memeriksa apakah sudah ada minimal satu admin */
static int hasAdmin(const UserList *list)
{
    int i;
    for (i = 0; i < list->count; i++) {
        if (list->data[i].role == ROLE_ADMIN) {
            return 1;
        }
    }
    return 0;
}

static void addDefaultAdmin(UserList *list)
{
    if (list->count >= MAX_USER) {
        return;
    }
    strcpy(list->data[list->count].username, DEFAULT_ADMIN_NAME);
    strcpy(list->data[list->count].password, DEFAULT_ADMIN_PASS);
    list->data[list->count].role = ROLE_ADMIN;
    list->count++;
}

void loadUsers(UserList *list)
{
    FILE *file = fopen(FILE_USERS, "r");
    char line[LINE_LEN];
    User temp;

    list->count = 0;
    if (file != NULL) {
        while (fgets(line, sizeof(line), file) != NULL && list->count < MAX_USER) {
            trimText(line);
            if (line[0] == '\0') {
                continue;
            }
            /* %31 = NAME_LEN - 1 dan PASS_LEN - 1, mencegah buffer overflow */
            if (sscanf(line, "%31[^|]|%31[^|]|%d", temp.username, temp.password, &temp.role) == 3) {
                if (temp.role != ROLE_ADMIN) {
                    temp.role = ROLE_USER;
                }
                list->data[list->count] = temp;
                list->count++;
            }
        }
        fclose(file);
    }

    if (!hasAdmin(list)) {
        addDefaultAdmin(list);
        saveUsers(list);
    }
}

int saveUsers(const UserList *list)
{
    FILE *file = fopen(FILE_USERS, "w");
    int i;

    if (file == NULL) {
        printError("Gagal menyimpan users.txt. Pastikan folder data/ ada.");
        return 0;
    }
    for (i = 0; i < list->count; i++) {
        fprintf(file, "%s|%s|%d\n", list->data[i].username,
                list->data[i].password, list->data[i].role);
    }
    fclose(file);
    return 1;
}

/* Sequential search pada array akun. Mengembalikan indeks atau -1. */
int findUserIndex(const UserList *list, const char *username)
{
    int i;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->data[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int validateUsername(const char *username)
{
    int length = (int) strlen(username);
    int i;

    if (length == 0) {
        printError("Username tidak boleh kosong.");
        return 0;
    }
    if (length < USERNAME_MIN || length > USERNAME_MAX) {
        printError("Username harus 3 sampai 20 karakter.");
        return 0;
    }
    for (i = 0; i < length; i++) {
        if (!isalnum((unsigned char) username[i]) && username[i] != '_') {
            printError("Username hanya boleh berisi huruf, angka, dan garis bawah (_).");
            return 0;
        }
    }
    return 1;
}

int validatePassword(const char *password)
{
    int length = (int) strlen(password);
    int hasLetter = 0;
    int hasDigit = 0;
    int i;

    if (length == 0) {
        printError("Password tidak boleh kosong.");
        return 0;
    }
    if (length < PASSWORD_MIN || length > PASSWORD_MAX) {
        printError("Password harus 6 sampai 20 karakter.");
        return 0;
    }
    for (i = 0; i < length; i++) {
        if (isspace((unsigned char) password[i]) || password[i] == '|') {
            printError("Password tidak boleh mengandung spasi atau karakter '|'.");
            return 0;
        }
        if (isalpha((unsigned char) password[i])) hasLetter = 1;
        if (isdigit((unsigned char) password[i])) hasDigit = 1;
    }
    if (!hasLetter || !hasDigit) {
        printError("Password harus mengandung minimal satu huruf dan satu angka.");
        return 0;
    }
    return 1;
}

int registerUser(UserList *list)
{
    User newUser;
    char confirmPassword[PASS_LEN];

    printBoxTitle("REGISTER AKUN BARU");
    printInfo("Username: 3-20 karakter (huruf, angka, atau _).");
    printInfo("Password: 6-20 karakter, wajib ada huruf dan angka.");
    printf("\n");

    if (list->count >= MAX_USER) {
        printError("Kapasitas akun sudah penuh.");
        return 0;
    }

    readString("  Username        : ", newUser.username, NAME_LEN);
    toLowerCase(newUser.username);   /* username tidak membedakan huruf besar/kecil */
    if (!validateUsername(newUser.username)) {
        return 0;
    }
    if (findUserIndex(list, newUser.username) != -1) {
        printError("Username sudah digunakan. Silakan pilih username lain.");
        return 0;
    }

    readString("  Password        : ", newUser.password, PASS_LEN);
    if (!validatePassword(newUser.password)) {
        return 0;
    }
    readString("  Ulangi Password : ", confirmPassword, PASS_LEN);
    if (strcmp(newUser.password, confirmPassword) != 0) {
        printError("Konfirmasi password tidak cocok.");
        return 0;
    }

    newUser.role = ROLE_USER;
    list->data[list->count] = newUser;
    list->count++;

    if (saveUsers(list)) {
        printSuccess("Registrasi berhasil. Silakan login.");
    }
    return 1;
}

/* Mengembalikan 1 jika login berhasil dan mengisi *loggedUser */
int loginUser(const UserList *list, User *loggedUser)
{
    char username[NAME_LEN];
    char password[PASS_LEN];
    char message[LINE_LEN];
    int index;

    printBoxTitle("LOGIN");
    printf("\n");

    if (readString("  Username : ", username, NAME_LEN) == 0) {
        printError("Username tidak boleh kosong.");
        return 0;
    }
    toLowerCase(username);
    if (readString("  Password : ", password, PASS_LEN) == 0) {
        printError("Password tidak boleh kosong.");
        return 0;
    }

    index = findUserIndex(list, username);
    if (index == -1 || strcmp(list->data[index].password, password) != 0) {
        printError("Login gagal. Username atau password salah.");
        return 0;
    }

    *loggedUser = list->data[index];
    snprintf(message, sizeof(message), "Login berhasil. Selamat datang, %s (%s).",
             loggedUser->username, roleToText(loggedUser->role));
    printSuccess(message);
    return 1;
}

const char *roleToText(int role)
{
    return (role == ROLE_ADMIN) ? "ADMIN" : "USER";
}
