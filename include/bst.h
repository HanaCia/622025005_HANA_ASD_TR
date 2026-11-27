/*
 * bst.h
 * Binary Search Tree sebagai indeks ID slot parkir.
 * Kunci  : ID slot (dibandingkan dengan strcmp).
 * Nilai  : indeks slot pada array parkir.
 * Dipakai untuk: cek ID unik saat tambah slot, cari slot berdasarkan ID,
 * hapus indeks saat slot dihapus, dan menampilkan ID terurut (in-order).
 */
#ifndef BST_H
#define BST_H

#include "config.h"

typedef struct BSTNode {
    char slotId[ID_LEN];
    int  slotIndex;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

BSTNode *bstInsert(BSTNode *root, const char *slotId, int slotIndex);
BSTNode *bstSearch(BSTNode *root, const char *slotId, int *steps);
BSTNode *bstDelete(BSTNode *root, const char *slotId);
BSTNode *bstFindMin(BSTNode *root);
void bstPreOrder(const BSTNode *root, int *counter);
void bstInOrder(const BSTNode *root, int *counter);
void bstPostOrder(const BSTNode *root, int *counter);
int  bstHeight(const BSTNode *root);
int  bstCountNodes(const BSTNode *root);
void bstShiftIndexes(BSTNode *root, int removedIndex);
void bstFree(BSTNode *root);

#endif
