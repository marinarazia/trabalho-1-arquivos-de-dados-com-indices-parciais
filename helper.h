#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include "entities.h"

float toDollars(const int cents);

ll* getHeadPointer(const char* dataFileName);

ll fseekBTreeOffset(FILE* dataFile, BPTree* tree, const long long id);

ll fseekSegmentOffset(FILE* dataFile, FILE* indexFile, const ll id);

void printProduct(const Product p);

void printOrder(const Order o);

Order* createNewOrder();

Product* createNewProduct();

#endif
