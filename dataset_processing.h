#ifndef INDEXER_H
#define INDEXER_H

#include <stddef.h>  // size_t
#include <stdio.h>   // FILE*

#include "entities.h"
#include "bpt_index.h"

int createIndex(const char *dataFile,
                const char *indexFile,
                const size_t recordSize,
                BPTree *bptree);

void reorganizeFile(const char* dataFile,
                    const char* indexFile,
                    const size_t recordSize,
                    BPTree *bptree);

void convertTextToBinary();

int hashFunction(ll key, int tableSize);
HashTable* createHashTable(int size);
void hashInsert(HashTable *ht, ll key, long offset);
HashTable* createHashIndex(const char *dataFile, int tableSize);
HashEntry* hashSearch(HashTable *ht, ll key);
void freeHashTable(HashTable *ht);

#endif
