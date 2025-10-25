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

    if (status.modificationsOrder)
    {
        reorganizeOrderFile();
    }

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
    FILE *dataFile = fopen(BIN_PRODUCT, "r+b");
    FILE *indexFile = fopen(INDEX_PRODUCT, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    if (status.modificationsProduct)
    {
        reorganizeProductFile();
    }

    ll segLastId = fseekSegmentOffset(dataFile, indexFile, productId);

    printf("Buscando produto ID %lld...\n", productId);

    Product p;
    int found = 0;
    int checked = 0;

    printf("\nSEGMENTO:\n");
    while (fread(&p, sizeof(Product), 1, dataFile) && p.id <= segLastId) 
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
    }
    else
    {
        printf("Produto ID %lld nao encontrado.\n", productId);
    }

    printf("Foram verificados %d registros.\n", checked);

    fclose(dataFile);
    return found;
}

int searchOrderById(const ll orderId)
{
    FILE *dataFile = fopen(BIN_ORDER, "r+b");
    FILE *indexFile = fopen(INDEX_ORDER, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    if (status.modificationsOrder)
    {
        reorganizeOrderFile();
    }

    ll segLastId = fseekSegmentOffset(dataFile, indexFile, orderId);

    printf("Buscando pedido ID %lld...\n", orderId);

    Order o;
    int found = 0;
    int checked = 0;

    printf("\nSEGMENTO:\n");
    while (fread(&o, sizeof(Order), 1, dataFile) && o.id <= segLastId)
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
