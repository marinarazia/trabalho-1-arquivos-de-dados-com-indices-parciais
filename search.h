#ifndef SEARCH_H
#define SEARCH_H

#include "entities.h"

void listOrders(const int limit);
void listProducts(const int limit);

int searchProductById(const ll productId);
int searchOrderById(const ll orderId);

int searchOrdersByUser(const ll userId);
int searchOrdersByUserHash(HashTable *ht, ll userId);

#endif
