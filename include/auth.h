/*
 * auth.h
 * Modul akun: register, login, dan penyimpanan akun ke users.txt.
 */
#ifndef AUTH_H
#define AUTH_H

#include "config.h"

typedef struct {
    char username[NAME_LEN];
    char password[PASS_LEN];
    int  role;               /* ROLE_ADMIN atau ROLE_USER */
} User;

typedef struct {
    User data[MAX_USER];     /* array akun */
    int  count;              /* jumlah akun terisi */
} UserList;

void loadUsers(UserList *list);
int  saveUsers(const UserList *list);
int  findUserIndex(const UserList *list, const char *username);
int  validateUsername(const char *username);
int  validatePassword(const char *password);
int  registerUser(UserList *list);
int  loginUser(const UserList *list, User *loggedUser);
const char *roleToText(int role);

#endif
