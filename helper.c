/*
static inline float toDollars(const int cents)
static ll* getHeadPointer(const char* dataFileName)
ll fseekSegmentOffset(FILE* dataFile, FILE* indexFile, const ll id)
void printProduct(const Product p)
void printOrder(const Order o)
Product* createNewProduct()
Order* createNewOrder()
*/

static inline float toDollars(const int cents)
{
    return cents / 100.0f;
}

ll* getHeadPointer(const char* dataFileName)
{
    if (strstr(dataFileName, PRODUCT_DAT))
        return &status.headProduct;
    if (strstr(dataFileName, ORDER_DAT))
        return &status.headOrder;
    return NULL;
}

// fseek into to the right segment based on the id with binary search
ll fseekSegmentOffset(FILE* dataFile, FILE* indexFile, const ll id)
{
    long blockOffset = 0;
    Index ie;
    long left = 0, right;
    long numEntries;

    fseek(indexFile, 0, SEEK_END);
    numEntries = ftell(indexFile) / sizeof(Index);
    if (numEntries == 0)
    {
        fseek(dataFile, 0, SEEK_SET);
        return -1;
    }
    right = numEntries - 1;

    long candidate = -1;

    while (left <= right)
    {
        long mid = left + (right - left) / 2;
        fseek(indexFile, mid * sizeof(Index), SEEK_SET);
        fread(&ie, sizeof(Index), 1, indexFile);

        if (ie.id >= id)
        {
            candidate = mid;
            right = mid - 1;
        }
        else 
        {
            left = mid + 1;
        }
    }

    fseek(indexFile, candidate * sizeof(Index), SEEK_SET);
    fread(&ie, sizeof(Index), 1, indexFile);
    blockOffset = ie.position;
    fseek(dataFile, blockOffset, SEEK_SET);

    //printf("%lld", ie.id);

    return ie.id;
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

Order* createNewOrder()
{
    Order *newOrder = malloc(sizeof(Order));
    if (!newOrder) return NULL;

    printf("Inserir novo pedido:\n");
    printf("ID do pedido: ");
    scanf("%lld", &newOrder->id);
    printf("ID do produto: ");
    scanf("%lld", &newOrder->purchasedProductId);
    printf("ID do usuario: ");
    scanf("%lld", &newOrder->userId);
    printf("Quantidade: ");
    scanf("%d", &newOrder->skuQty);

    newOrder->dateTime = time(NULL);
    newOrder->active = '1';
    newOrder->next = -1;

    return newOrder;
}

Product* createNewProduct()
{
    Product *newProduct = malloc(sizeof(Product));
    if (!newProduct) return NULL;

    float price;
    printf("Inserir novo produto:\n");
    printf("ID do produto: ");
    scanf("%lld", &newProduct->id);
    /*
    printf("ID da categoria: ");
    scanf("%lld", &newProduct->categoryId);
    printf("Alias da categoria: ");
    scanf("%s", newProduct->categoryAlias);
    printf("ID da marca: ");
    scanf("%lld", &newProduct->brandId);
    printf("Preco (ex: 29.99): ");
    scanf("%f", &price);
    newProduct->price = (int)(price * 100);
    getchar();
    printf("Genero (M/F/U): ");
    scanf(" %c", &newProduct->productGender);
    */

    newProduct->active = '1';
    newProduct->next = -1;

    return newProduct;
}
