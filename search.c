/*
void listOrders(const int limit);
void listProducts(const int limit);
int searchProductById(const ll productId);
int searchOrdersByUser(const ll userId);
*/

void listOrders(const int limit) {
    FILE *binOrder = fopen(BIN_ORDER, "rb");
    if (!binOrder) 
    { 
        printf("Arquivo orders.bin nao encontrado.\n"); 
        return; 
    }

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
    
    Index idx;
    Index prevIdx = {0, 0};
    
    while (fread(&idx, sizeof(Index), 1, indexFile)) 
	{
        if (idx.key > productId) 
		{
            break;
        }
        
        prevIdx = idx;
    }
    
    fseek(dataFile, prevIdx.segmentBase, SEEK_SET);
    
    Product p;
    while (fread(&p, sizeof(Product), 1, dataFile)) 
	{
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
