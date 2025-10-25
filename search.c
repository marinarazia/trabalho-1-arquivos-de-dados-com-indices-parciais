/*
void listOrders(const int limit);
void listProducts(const int limit);
int searchProductById(const ll productId);
int searchOrdersByUser(const ll userId);
int searchOrderByIdWithExtension(const ll orderId);
long findOrderPosition(ll orderId);
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
            break;
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

        printf("ID: %lld | ProdutoID: %lld | UsuarioID: %lld | Qtd: %d | Data: %s | Elo: %lld\n",
               o.id, o.purchasedProductId, o.userId, o.skuQty, buffer, o.next);

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

        printf("ProdutoID: %lld | MarcaID: %lld | Preco: %.2f | CategoriaID: %lld | CategoriaAlias: %s | Genero: %c | Elo: %lld\n",
               p.id,
               p.brandId,
               p.price / 100.0,
               p.categoryId,
               p.categoryAlias,
               p.productGender,
			   p.next);

        if (limit && ++count >= limit) break;
    }
    fclose(binProduct);
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
            printf("ID: %lld\n", p.id);
            printf("Categoria: %s (ID: %lld)\n", p.categoryAlias, p.categoryId);
            printf("Marca ID: %lld\n", p.brandId);
            printf("Preco: $%.2f\n", p.price / 100.0);
            printf("Genero: %c\n", p.productGender);
            printf("Proximo elo: %lld\n", p.next);
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

    printf("Buscando pedido ID %lld...\n", orderId);

    Order o;
    int found = 0;
    int mainFound = 0;

    fseek(dataFile, 0, SEEK_SET);
    while (fread(&o, sizeof(Order), 1, dataFile) && !mainFound) {
        if (o.active != '0' && o.id == orderId) {
            mainFound = 1;
            found = 1;
            
            char buffer[32];
            struct tm *tm_info = localtime(&o.dateTime);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
            
            printf("\nPEDIDO ENCONTRADO\n");
            printf("ID: %lld\n", o.id);
            printf("Usuario: %lld\n", o.userId);
            printf("Produto: %lld\n", o.purchasedProductId);
            printf("Quantidade: %d\n", o.skuQty);
            printf("Data: %s\n", buffer);
            printf("Proximo elo: %lld\n", o.next);
            
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
