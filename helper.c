static inline float toDollars(const int cents)
{
    return cents / 100.0f;
}

void printProduct(const Product p)
{
    printf("ProdutoID: %lld | MarcaID: %lld | Preco: %.2f | CategoriaID: %lld | CategoriaAlias: %s | Genero: %c | Elo: %lld\n",
           p.id,
           p.brandId,
           toDollars(p.price),
           p.categoryId,
           p.categoryAlias,
           p.productGender,
           p.next);
}

void printOrder(const Order o)
{
    char buffer[32];
    struct tm *tm_info = localtime(&o.dateTime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("ID: %lld | ProdutoID: %lld | UsuarioID: %lld | Qtd: %d | Data: %s | Elo: %lld\n",
           o.id, 
           o.purchasedProductId, 
           o.userId, 
           o.skuQty, 
           buffer, 
           o.next);
}

//binary search in indexFile with id, fseek result in dataFile - Ricardo
ll fseekBlockOffset(FILE* dataFile, FILE* indexFile, const ll id)
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

        if (ie.id <= id)
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

    return ie.id;
}
