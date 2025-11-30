#ifndef BPT_INDEX_H
#define BPT_INDEX_H

#include "entities.h"
#include "config.h"

BPTree bpt_create();
BPTNode* bpt_create_leaf();

void bpt_insert(BPTree *tree, long long key, long offset);
long bpt_search(BPTree *tree, long long key);

BPTNode* bpt_find_leaf(BPTree *tree, long long key);
void bpt_insert_in_leaf(BPTNode *leaf, long long key, long offset);
void bpt_insert_in_parent(BPTree *tree, BPTNode *left, long long key, BPTNode *right);
void bpt_split_leaf(BPTree *tree, BPTNode *leaf);

#endif
