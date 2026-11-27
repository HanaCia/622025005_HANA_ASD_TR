/*
 * bst.c
 * Implementasi Binary Search Tree secara rekursif.
 * Aturan BST: semua ID di subtree kiri lebih kecil dari node,
 * semua ID di subtree kanan lebih besar dari node.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bst.h"
#include "display.h"

#define TRAVERSAL_PER_LINE 8   /* jumlah ID per baris saat traversal dicetak */

static BSTNode *createNode(const char *slotId, int slotIndex)
{
    BSTNode *node = (BSTNode *) malloc(sizeof(BSTNode));
    if (node == NULL) {
        return NULL;
    }
    snprintf(node->slotId, ID_LEN, "%s", slotId);
    node->slotIndex = slotIndex;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Insert: turun ke kiri jika lebih kecil, ke kanan jika lebih besar */
BSTNode *bstInsert(BSTNode *root, const char *slotId, int slotIndex)
{
    int result;

    if (root == NULL) {
        return createNode(slotId, slotIndex);
    }
    result = strcmp(slotId, root->slotId);
    if (result < 0) {
        root->left = bstInsert(root->left, slotId, slotIndex);
    } else if (result > 0) {
        root->right = bstInsert(root->right, slotId, slotIndex);
    }
    /* result == 0: ID sudah ada, tidak dimasukkan dua kali */
    return root;
}

/* Search: mengembalikan node yang ditemukan atau NULL. steps boleh NULL. */
BSTNode *bstSearch(BSTNode *root, const char *slotId, int *steps)
{
    int result;

    while (root != NULL) {
        if (steps != NULL) (*steps)++;
        result = strcmp(slotId, root->slotId);
        if (result == 0) {
            return root;
        }
        root = (result < 0) ? root->left : root->right;
    }
    return NULL;
}

/* Node paling kiri = ID terkecil pada subtree */
BSTNode *bstFindMin(BSTNode *root)
{
    while (root != NULL && root->left != NULL) {
        root = root->left;
    }
    return root;
}

/*
 * Delete memiliki tiga kasus:
 * 1. Node daun (tanpa anak)     : langsung dihapus.
 * 2. Node dengan satu anak      : anaknya naik menggantikan node.
 * 3. Node dengan dua anak       : isi node diganti dengan successor
 *                                 (ID terkecil di subtree kanan), lalu
 *                                 successor dihapus dari subtree kanan.
 */
BSTNode *bstDelete(BSTNode *root, const char *slotId)
{
    BSTNode *temp;
    int result;

    if (root == NULL) {
        return NULL;
    }
    result = strcmp(slotId, root->slotId);
    if (result < 0) {
        root->left = bstDelete(root->left, slotId);
    } else if (result > 0) {
        root->right = bstDelete(root->right, slotId);
    } else {
        if (root->left == NULL) {           /* kasus 1 dan 2 */
            temp = root->right;
            free(root);
            return temp;
        }
        if (root->right == NULL) {          /* kasus 2 */
            temp = root->left;
            free(root);
            return temp;
        }
        temp = bstFindMin(root->right);     /* kasus 3 */
        snprintf(root->slotId, ID_LEN, "%s", temp->slotId);
        root->slotIndex = temp->slotIndex;
        root->right = bstDelete(root->right, root->slotId);
    }
    return root;
}

static void printNodeId(const BSTNode *node, int *counter)
{
    if (*counter % TRAVERSAL_PER_LINE == 0) {
        printf("\n    ");
    }
    printf("%s%-7s%s ", clr(CLR_YELLOW), node->slotId, clr(CLR_RESET));
    (*counter)++;
}

/* Pre-order: node -> kiri -> kanan */
void bstPreOrder(const BSTNode *root, int *counter)
{
    if (root == NULL) return;
    printNodeId(root, counter);
    bstPreOrder(root->left, counter);
    bstPreOrder(root->right, counter);
}

/* In-order: kiri -> node -> kanan (hasilnya ID terurut naik) */
void bstInOrder(const BSTNode *root, int *counter)
{
    if (root == NULL) return;
    bstInOrder(root->left, counter);
    printNodeId(root, counter);
    bstInOrder(root->right, counter);
}

/* Post-order: kiri -> kanan -> node */
void bstPostOrder(const BSTNode *root, int *counter)
{
    if (root == NULL) return;
    bstPostOrder(root->left, counter);
    bstPostOrder(root->right, counter);
    printNodeId(root, counter);
}

int bstHeight(const BSTNode *root)
{
    int leftHeight, rightHeight;
    if (root == NULL) return 0;
    leftHeight = bstHeight(root->left);
    rightHeight = bstHeight(root->right);
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

int bstCountNodes(const BSTNode *root)
{
    if (root == NULL) return 0;
    return 1 + bstCountNodes(root->left) + bstCountNodes(root->right);
}

/* Setelah slot dihapus dari array, indeks di belakangnya bergeser satu */
void bstShiftIndexes(BSTNode *root, int removedIndex)
{
    if (root == NULL) return;
    if (root->slotIndex > removedIndex) {
        root->slotIndex--;
    }
    bstShiftIndexes(root->left, removedIndex);
    bstShiftIndexes(root->right, removedIndex);
}

/* Post-order dipakai saat free: anak dibebaskan dulu sebelum induknya */
void bstFree(BSTNode *root)
{
    if (root == NULL) return;
    bstFree(root->left);
    bstFree(root->right);
    free(root);
}
