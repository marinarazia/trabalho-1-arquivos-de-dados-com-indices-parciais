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
        printf("Arquivo orders.bin não encontrado!\n"); 
        return; 
    }

    Order o;
    int count = 0;
    printf("\nORDERS:\n");
    while (fread(&o, sizeof(Order), 1, binOrder)) 
    {
        char buffer[32];
        struct tm *tm_info = localtime(&o.dateTime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("ID: %lld | ProdutoID: %lld | UsuárioID: %lld | Qtd: %d | Data: %s\n",
               o.id, o.purchasedProductId, o.userId, o.skuQty, buffer);

        if (limit && ++count >= limit) break;
    }
    fclose(binOrder);
}

void listProducts(const int limit) {
    FILE *binProduct = fopen(BIN_PRODUCT, "rb");
    if (!binProduct) 
    { 
        printf("Arquivo products.bin não encontrado!\n"); 
        return; 
    }

    Product p;
    int count = 0;
    printf("\nPRODUCTS:\n");
    while (fread(&p, sizeof(Product), 1, binProduct))
    {
        printf("ProdutoID: %lld | MarcaID: %lld | Preço: %.2f | CategoriaID: %lld | CategoriaAlias: %s | Gênero: %c\n",
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

//TODO: completar métodos ou adicionar funções de consulta diferentes
int searchProductById(const ll productId)
{
   return 0; 
}

int searchOrdersByUser(const ll userId)
{
    return 0;
}
