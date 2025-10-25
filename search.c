/*
void listOrders(const int limit);
void listProducts(const int limit);
int searchOrdersByUser(const ll userId);
int searchProductById(const ll productId);
int searchOrderById(const ll orderId);
*/

void listOrders(const int limit)
{
    FILE *binOrder = fopen(BIN_ORDER, "rb");
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
    FILE *binProduct = fopen(BIN_PRODUCT, "rb");
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

// scan dataFile por não ter index de usuário
int searchOrdersByUser(const ll userId)
{
    FILE *dataFile = fopen(BIN_ORDER, "rb");
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

int searchProductById(const ll productId)
{
    FILE *dataFile = fopen(BIN_PRODUCT, "rb");
    if (!dataFile) return 0;

    printf("Buscando produto ID %lld...\n", productId);

    Product p;
    int found = 0;
    int checked = 0;

    while (fread(&p, sizeof(Product), 1, dataFile)) 
	{
        checked++;
        
        if (p.active == '0') continue;

        if (p.id == productId) 
		{
            printf("\nPRODUTO ENCONTRADO\n");
            printProduct(p);
            found = 1;
            break;
        }
    }

    if (!found) 
	{
        printf("Produto ID %lld nao encontrado.\n", productId);
        printf("Foram verificados %d registros.\n", checked);
    }

    fclose(dataFile);
    return found;
}

int searchOrderById(const ll orderId)
{
    FILE *dataFile = fopen(BIN_ORDER, "rb");
    if (!dataFile) return 0;

    printf("Buscando pedido ID %lld...\n", orderId);

    Order o;
    int found = 0;
    int mainFound = 0;

    fseek(dataFile, 0, SEEK_SET);
    while (fread(&o, sizeof(Order), 1, dataFile) && !mainFound) {
        if (o.active != '0' && o.id == orderId) {
            mainFound = 1;
            found = 1;
            
            printf("\nPEDIDO ENCONTRADO\n");
            printOrder(o);
            
            if (o.next != -1) {
                printf("\nSEGUINDO CADEIA DE EXTENSAO\n");
                Order current = o;
                int chainLevel = 1;
                
                while (current.next != -1 && chainLevel < 10) 
				{
                    fseek(dataFile, 0, SEEK_SET);
                    int foundNext = 0;
                    Order nextOrder;
                    
                    while (fread(&nextOrder, sizeof(Order), 1, dataFile)) 
					{
                        if (nextOrder.active != '0' && nextOrder.id == current.next) 
						{
                            current = nextOrder;
                            foundNext = 1;
                            
                            printf("Elo %d: ID %lld -> Usuario: %lld, Produto: %lld, Proximo Elo: %lld\n",
                                   chainLevel, current.id, current.userId, 
                                   current.purchasedProductId, current.next);
                            break;
                        }
                    }
                    
                    if (!foundNext) {
                        printf("Elo %lld nao encontrado.\n", current.next);
                        break;
                    }
                    
                    chainLevel++;
                }
                
                if (chainLevel >= 10) {
                    printf("Cadeia muito longa, parando por segurança.\n");
                }
            }
        }
    }

    if (!found) {
        printf("Pedido ID %lld nao encontrado.\n", orderId);
    }

    fclose(dataFile);
    return found;
}
