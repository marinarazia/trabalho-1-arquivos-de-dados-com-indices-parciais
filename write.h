#ifndef DATASET_ENTITIES_H
#define DATASET_ENTITIES_H

#include "bpt_index.h"
#include "entities.h"
#include <stddef.h>

int insert(const char* dataFileName,
           const char* indexFileName,
           const void* record,
           size_t recordSize,
           BPTree* btree);

int writeRecord(const char* dataFileName,
                const char* indexFileName,
                const void* updatedRecord,
                size_t recordSize,
                long long id,
                BPTree* btree);

int removeProduct(long long productId);
int removeOrder(long long orderId);

#endif
