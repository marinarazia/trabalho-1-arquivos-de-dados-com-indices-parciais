/*
void listOrders(const int limit);
void listProducts(const int limit);
int searchProductById(const ll productId);
int searchOrdersByUser(const ll userId);
int searchOrderByIdWithExtension(const ll orderId);
long findOrderPosition(ll orderId);
void reorganizeOrderFile();
*/

long findOrderPosition(ll orderId)
{
    FILE *file = fopen(BIN_ORDER, "rb");
    if (!file) return -1;

    Order o;
    long pos = 0;

    while (fread(&o, sizeof(Order), 1, file))
    {
        if (o.id == orderId && o.active != '0')
        {
            fclose(file);
            return pos;
        }
        pos = ftell(file);
    }

    fclose(file);
    return -1;
}

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

        char buffer[32];
        struct tm *tm_info = localtime(&o.dateTime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("ID: %lld | ProdutoID: %lld | UsuarioID: %lld | Qtd: %d | Data: %s\n",
               o.id, o.purchasedProductId, o.userId, o.skuQty, buffer);

        if (limit && ++count >= limit) break;
    }
    fclose(binOrder);
}

void listProducts(const int limit) {
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

        printf("ProdutoID: %lld | MarcaID: %lld | Preco: %.2f | CategoriaID: %lld | CategoriaAlias: %s | Genero: %c\n",
               p.purchasedProductId,
               p.brandId,
               p.price / 100.0,
               p.categoryId,
               p.categoryAlias,
               p.productGender);

        if (limit && ++count >= limit) break;
    }
    fclose(binProduct);
}

int searchProductById(const ll productId)
{
    FILE *indexFile = fopen(INDEX_PRODUCT, "rb");
    FILE *dataFile = fopen(BIN_PRODUCT, "rb");

    if (!indexFile || !dataFile) return 0;

    fseek(indexFile, 0, SEEK_END);
    long indexSize = ftell(indexFile);
    long numIndexEntries = indexSize / sizeof(Index);

    long left = 0;
    long right = numIndexEntries - 1;
    long mid;
    Index idx;
    Index bestIdx = {0, 0};

    while (left <= right)
    {
        mid = left + (right - left) / 2;

        fseek(indexFile, mid * sizeof(Index), SEEK_SET);
        fread(&idx, sizeof(Index), 1, indexFile);

        if (idx.key == productId)
        {
            bestIdx = idx;
            break;
        }
        else if (idx.key < productId)
        {
            bestIdx = idx;
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    fseek(dataFile, bestIdx.position, SEEK_SET);

    Product p;
    int found = 0;
    long recordsChecked = 0;
    while (fread(&p, sizeof(Product), 1, dataFile) && recordsChecked < SEGMENT_SIZE)
	{
	    if (p.active == '0') continue;

        if (p.purchasedProductId == productId)
		{
            printf("Produto encontrado: ID %lld | Preco: %.2f | Categoria: %s\n",
                   p.purchasedProductId, p.price/100.0, p.categoryAlias);
            fclose(indexFile);
            fclose(dataFile);
            return 1;
        }
        if (p.purchasedProductId > productId) break;
    }

    printf("Produto nao encontrado.\n");
    fclose(indexFile);
    fclose(dataFile);
    return 0;
}

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
            char buffer[32];
            struct tm *tm_info = localtime(&o.dateTime);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

            printf("Pedido ID: %lld | Produto: %lld | Data: %s | Qtd: %d\n",
                   o.id, o.purchasedProductId, buffer, o.skuQty);
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

int searchOrderByIdWithExtension(const ll orderId)
{
    FILE *dataFile = fopen(BIN_ORDER, "rb");
    if (!dataFile) return 0;

    fseek(dataFile, 0, SEEK_END);
    long fileSize = ftell(dataFile);
    long numRecords = fileSize / sizeof(Order);

    long left = 0;
    long right = numRecords - 1;
    Order o;
    int found = 0;
    long foundPos = -1;

    while (left <= right && !found)
    {
        long mid = left + (right - left) / 2;

        fseek(dataFile, mid * sizeof(Order), SEEK_SET);
        if (fread(&o, sizeof(Order), 1, dataFile) != 1) break;

        if (o.active == '0') continue;

        if (o.id == orderId)
        {
            found = 1;
            foundPos = mid * sizeof(Order);
        }
        else if (o.id < orderId)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    if (!found)
    {
        fseek(dataFile, 0, SEEK_SET);
        while (fread(&o, sizeof(Order), 1, dataFile) && !found)
        {
            if (o.active == '0') continue;

            Order current = o;
            while (current.next != -1 && !found)
            {
                long extPos = findOrderPosition(current.next);
                if (extPos == -1) break;

                fseek(dataFile, extPos, SEEK_SET);
                if (fread(&current, sizeof(Order), 1, dataFile) != 1) break;

                if (current.id == orderId && current.active != '0')
                {
                    foundPos = extPos;
                    o = current;
                    char buffer[32];
                    struct tm *tm_info = localtime(&o.dateTime);
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

                    printf("Pedido ID: %lld | Usuario ID: %lld | Produto ID: %lld | Quantidade: %d | Data: %s | Proximo elo: %lld | Posicao: %ld"
                           , o.id, o.userId,o.purchasedProductId, o.skuQty, buffer, o.next, foundPos);

                    if (o.next != -1)
                    {
                        printf("\nCADEIA DE EXTENSAO:\n");
                        Order current = o;
                        while (current.next != -1)
                        {
                            long extPos = findOrderPosition(current.next);
                            if (extPos == -1) break;

                            fseek(dataFile, extPos, SEEK_SET);
                            if (fread(&current, sizeof(Order), 1, dataFile) != 1) break;

                            if (current.active != '0')
                            {
                                printf("Elo: ID %lld -> Proximo: %lld\n",
                                       current.id, current.next);
                            }
                        }
                    }
                    found++;
                    break;
                }
            }

            if (found) break;
            else
            {
                printf("Pedido ID %lld nao encontrado.\n", orderId);
            }
        }
    }

    fclose(dataFile);
    return found;
}

void reorganizeOrderFile()
{
    printf("Reorganizando arquivo de pedidos...\n");
    
    FILE *oldFile = fopen(BIN_ORDER, "rb");
    FILE *newFile = fopen("temp_orders_reorg.bin", "wb");
    
    if (!oldFile || !newFile) {
        printf("Erro ao reorganizar arquivo.\n");
        return;
    }

    Order o;
    int i;
    int count = 0;
    int movedFromExtension = 0;
    
    Order activeOrders[10000]; 
    int activeCount = 0;
    
    while (fread(&o, sizeof(Order), 1, oldFile)) 
	{
        if (o.active != '0') 
		{
            o.next = -1;
            activeOrders[activeCount++] = o;
            
            if (o.id >= EXTENSION_AREA_START) 
			{
                movedFromExtension++;
            }
        }
    }

    qsort(activeOrders, activeCount, sizeof(Order), compareOrder);
    
    for (i = 0; i < activeCount; i++) 
	{
        fwrite(&activeOrders[i], sizeof(Order), 1, newFile);
        count++;
    }
    
    fclose(oldFile);
    fclose(newFile);
    
    remove(BIN_ORDER);
    rename("temp_orders_reorg.bin", BIN_ORDER);
    
    currentExtensionId = EXTENSION_AREA_START;
    
    printf("Reorganizacao concluida: %d registros, %d movidos da extensao\n", 
           count, movedFromExtension);
    
    createIndex();
}
