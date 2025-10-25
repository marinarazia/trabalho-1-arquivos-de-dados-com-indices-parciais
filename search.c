/*
int fseekBlockOffset(FILE* dataFile, FILE* indexFile, const ll id)
void listOrders(const int limit);
void listProducts(const int limit);
int searchProductById(const ll productId);
int searchOrdersByUser(const ll userId);
int searchOrderByIdWithExtension(const ll orderId);
long findOrderPosition(ll orderId);
*/

int fseekBlockOffset(FILE* dataFile, FILE* indexFile, const ll id)
{
    long blockOffset = 0;
    Index ie;
    long left = 0, right;
    long numEntries;

    fseek(indexFile, 0, SEEK_END);
    numEntries = ftell(indexFile) / sizeof(Index);
    right = numEntries - 1;

    long candidate = -1;
    while (left <= right)
    {
        long mid = left + (right - left) / 2;
        fseek(indexFile, mid * sizeof(Index), SEEK_SET);
        fread(&ie, sizeof(Index), 1, indexFile);

        if (ie.key <= id)
        {
            candidate = mid;
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    if (candidate != -1)
    {
        fseek(indexFile, candidate * sizeof(Index), SEEK_SET);
        fread(&ie, sizeof(Index), 1, indexFile);
        blockOffset = ie.position;
    }

    fseek(dataFile, blockOffset, SEEK_SET);
}


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

        printf("ID: %lld | ProdutoID: %lld | UsuarioID: %lld | Qtd: %d | Data: %s | Elo: %lld \n", 
               o.id, 
               o.purchasedProductId, 
               o.userId, 
               o.skuQty, 
               buffer, 
               o.next);

        #ifndef DEBUG
        if (limit && ++count >= limit) break;
        #endif
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

        printf("ProdutoID: %lld | MarcaID: %lld | Preco: %.2f | CategoriaID: %lld | CategoriaAlias: %s | Genero: %c | Elo: %lld\n",
               p.id,
               p.brandId,
               p.price * 100.0,
               p.categoryId,
               p.categoryAlias,
               p.productGender,
               p.next);

        #ifndef DEBUG
        if (limit && ++count >= limit) break;
        #endif
    }
    fclose(binProduct);
}

int searchProductById(const ll product)
{
    FILE *dataFile = fopen(BIN_PRODUCT, "r+b");
    FILE *indexFile = fopen(INDEX_PRODUCT, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    long blockOffset = 0;
    Index ie;
    long left = 0, right;
    long numEntries;

    fseek(indexFile, 0, SEEK_END);
    numEntries = ftell(indexFile) / sizeof(Index);
    right = numEntries - 1;

    long candidate = -1;
    while (left <= right)
    {
        long mid = left + (right - left) / 2;
        fseek(indexFile, mid * sizeof(Index), SEEK_SET);
        fread(&ie, sizeof(Index), 1, indexFile);

        if (ie.key <= product)
        {
            candidate = mid;
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    blockOffset = 0;
    if (candidate != -1)
    {
        fseek(indexFile, candidate * sizeof(Index), SEEK_SET);
        fread(&ie, sizeof(Index), 1, indexFile);
        blockOffset = ie.position;
    }

    fseek(dataFile, blockOffset, SEEK_SET);
    Product current, previous;
    long currentPos = ftell(dataFile);
    long previousPos = -1;
    int found = -1;

    while (fread(&current, sizeof(Product), 1, dataFile))
    {
        if (current.active == '0')
        {
            previousPos = currentPos;
            currentPos = ftell(dataFile);
            continue;
        }

        if (current.id == product)
        {
            found = 1;
            break;
        }

        if (current.id > product) 
        {
            fseek(dataFile, previousPos, SEEK_SET);
            fseek(dataFile, previous.next, SEEK_SET);;
            fread(&current, sizeof(Product), 1, dataFile);

            while (current.id != product && current.next != -1)
            {
                fseek(dataFile, current.next, SEEK_SET);;
                fread(&current, sizeof(Product), 1, dataFile);
            }

            if (current.id == product)
            {
                found = 1;
            }

            break;
        }

        previousPos = currentPos;
        currentPos = ftell(dataFile);
    }

    if (found)
    {
        printf("ACHOU");
    }

    fclose(dataFile);
    fclose(indexFile);

    return 1;
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
            printf("Pedido ID: %lld | Usuario ID: %lld | Produto ID: %lld | Quantidade: %d" , o.id, o.userId,o.purchasedProductId, o.skuQty);
            break;
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
                break;
            }
        }
    }

    fclose(dataFile);
    return found;
}
