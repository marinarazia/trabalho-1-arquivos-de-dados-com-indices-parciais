/*
void listOrders(const int limit);
void listProducts(const int limit);
int  searchProductById(const ll productId);
int  searchOrderById(const ll orderId);
int  searchOrdersByUser(const ll userId);
*/

#include "dataset_processing.h"
#include "entities.h"
#include "helper.h"
#include <stdlib.h>
#include <stdio.h>

void listOrders(const int limit)
{
    FILE *binOrder = fopen(ORDER_DAT, "rb");
    if (!binOrder) return;

    Order o;
    int count = 0;
    printf("\nORDERS:\n");
    while (fread(&o, sizeof(Order), 1, binOrder))
    {
    	if (o.active == '0') continue;

        printOrder(o);

        if (limit && ++count >= limit) break;
    }
    fclose(binOrder);
}

void listProducts(const int limit)
{
    FILE *binProduct = fopen(PRODUCT_DAT, "rb");
    if (!binProduct)
    {
        printf("Arquivo products.bin nao encontrado.\n");
        return;
    }

    Product p;
    int count = 0;
    printf("\nPRODUCTS:\n");
    while (fread(&p, sizeof(Product), 1, binProduct))
    {
    	if (p.active == '0') continue;

        printProduct(p);

        if (limit && ++count >= limit) break;
    }
    fclose(binProduct);
}

int searchOrdersByUserHash(HashTable *ht, ll userId) 
{
    if (!ht) 
    {
        printf("Indice hash nao carregado.\n");
        return 0;
    }
    
    FILE *dataFile = fopen(ORDER_DAT, "rb");
    if (!dataFile) 
    {
        printf("Erro ao abrir arquivo de ordens.\n");
        return 0;
    }
    
    clock_t start = clock();
    
    HashEntry *entry = hashSearch(ht, userId);
    int found = 0;
    
    if (entry)
    {
        printf("PEDIDOS ENCONTRADOS (via Hash) para usuario %lld:\n", userId);
        
        // Buscar todos os registros com a mesma chave (resolução de colisões)
        HashEntry *current = ht->table[hashFunction(userId, ht->size)];
        while (current != NULL) 
        {
            if (current->key == userId) 
            {
                fseek(dataFile, current->file_offset, SEEK_SET);
                Order o;
                if (fread(&o, sizeof(Order), 1, dataFile)) 
                {
                    if (o.active != '0' && o.userId == userId) 
                    {
                        printOrder(o);
                        found++;
                    }
                }
            }
            current = current->next;
        }
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    if (!found) {
        printf("Nenhum pedido encontrado para o usuario %lld\n", userId);
    } else {
        printf("Tempo de busca com hash: %.4f segundos\n", time_spent);
    }
    
    fclose(dataFile);
    return found;
}

int searchProductById(const ll productId)
{
    if (status.modificationsProduct)
    {
        reorganizeFile(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product), &productTree);
    }

    FILE *dataFile = fopen(PRODUCT_DAT, "r+b");
    FILE *indexFile = fopen(PRODUCT_INDEX, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

#if USE_BTREE_INDEX
    fseekBTreeOffset(dataFile, &productTree, productId);
#else
    fseekSegmentOffset(dataFile, indexFile, productId);
#endif

    printf("Buscando produto ID %lld...\n", productId);

    Product p;
    int found = 0;
    int checked = 0;
    printf("\nSEGMENTO:\n");
    while (fread(&p, sizeof(Product), 1, dataFile) && checked < SEGMENT_SIZE) 
	{
        checked++;
        
        printProduct(p);

        if (p.active == '0') continue;

        if(p.id > productId)
        {
            fseek(dataFile, -1 * sizeof(Product), SEEK_SET);
            while (fread(&p, sizeof(Product), 1, dataFile) && p.next != -1)
            {
                fseek(dataFile, p.next, SEEK_SET); 
            }
        }

        if (p.id == productId) 
		{
            found = 1;
            break;
        }
    }

    if (found) 
	{
        printf("\nPRODUTO ENCONTRADO\n");
        printProduct(p);
        return found;
    }

    printf("Produto ID %lld nao encontrado.\n", productId);
    printf("Foram verificados %d registros.\n", checked);
    fclose(dataFile);
    return found;
}

int searchOrderById(const ll orderId)
{
    if (status.modificationsOrder)
    {
		reorganizeFile(ORDER_DAT, ORDER_INDEX, sizeof(Order), &orderTree);
    }

    FILE *dataFile = fopen(ORDER_DAT, "r+b");
    FILE *indexFile = fopen(ORDER_INDEX, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

#if USE_BTREE_INDEX
    fseekBTreeOffset(dataFile, &orderTree, orderId);
#else
    fseekSegmentOffset(dataFile, indexFile, orderId);
#endif

    printf("Buscando pedido ID %lld...\n", orderId);

    Order o;
    int found = 0;
    int checked = 0;
    printf("\nSEGMENTO:\n");
    while (fread(&o, sizeof(Order), 1, dataFile) && checked < SEGMENT_SIZE)
    {
        checked++;
        printOrder(o);

        if (o.active == '0') continue;

        if (o.id > orderId)
        {
            fseek(dataFile, -1 * sizeof(Order), SEEK_CUR);
            while (fread(&o, sizeof(Order), 1, dataFile) && o.next != -1)
            {
                fseek(dataFile, o.next, SEEK_SET);
            }
        }

        if (o.id == orderId)
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        printf("\nPEDIDO ENCONTRADO\n");
        printOrder(o);
    }
    else
    {
        printf("Pedido ID %lld nao encontrado.\n", orderId);
    }

    printf("Foram verificados %d registros.\n", checked);

    fclose(dataFile);
    return found;
}

// scan dataFile por n�o ter index de usu�rio
int searchOrdersByUser(const ll userId)
{
    FILE *dataFile = fopen(ORDER_DAT, "rb");
    if (!dataFile) return 0;

    Order o;
    int found = 0;
    while (fread(&o, sizeof(Order), 1, dataFile))
    {
        if (o.userId == userId)
        {
            printOrder(o);
            found++;
        }
    }

    if (!found)
    {
        printf("Nenhum pedido encontrado para o usuario %lld\n", userId);
    }

    fclose(dataFile);
    return found;
}
