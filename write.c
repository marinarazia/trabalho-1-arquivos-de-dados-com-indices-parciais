/*
int insertOrder(const Order order);
int insertProduct(const Product product);
int removeProduct(const ll productId);
int removeOrder(const ll orderId);
int insertOrderWithExtension(const Order order);
*/

int insertOrder(const Order order)
{
    FILE *file = fopen(BIN_ORDER, "r+b");
    if (!file) return 0;

    Order newOrder = order;
    newOrder.active = '1';
    newOrder.next = -1;

    if (order.id >= EXTENSION_AREA_START)
    {
        fseek(file, 0, SEEK_END);
        fwrite(&newOrder, sizeof(Order), 1, file);
        fclose(file);
        printf("Pedido ID %lld inserido na area de extensao.\n", order.id);
        return 1;
    }

    Order current, previous;
    long currentPos = 0, previousPos = -1;
    int inserted = 0;

    while (fread(&current, sizeof(Order), 1, file))
    {
        if (current.active == '0')
        {
            currentPos = ftell(file);
            continue;
        }

        if (current.id > order.id)
        {
            if (previousPos == -1)
            {
                fseek(file, 0, SEEK_SET);
            }
            else
            {
                fseek(file, previousPos, SEEK_SET);
                fread(&previous, sizeof(Order), 1, file);

                if (previous.next != -1)
                {
                    long extensionPos = ftell(file);
                    Order extension;
                    while (previous.next != -1)
                    {
                        extensionPos = findOrderPosition(previous.next);
                        if (extensionPos == -1) break;

                        fseek(file, extensionPos, SEEK_SET);
                        fread(&extension, sizeof(Order), 1, file);
                        previous = extension;
                    }

                    newOrder.id = currentExtensionId++;
                    fseek(file, 0, SEEK_END);
                    long newPos = ftell(file);
                    fwrite(&newOrder, sizeof(Order), 1, file);

                    previous.next = newOrder.id;
                    fseek(file, extensionPos, SEEK_SET);
                    fwrite(&previous, sizeof(Order), 1, file);

                    inserted = 1;
                    break;
                }
                else
                {
                    newOrder.id = currentExtensionId++;
                    fseek(file, 0, SEEK_END);
                    long newPos = ftell(file);
                    fwrite(&newOrder, sizeof(Order), 1, file);

                    previous.next = newOrder.id;
                    fseek(file, previousPos, SEEK_SET);
                    fwrite(&previous, sizeof(Order), 1, file);

                    inserted = 1;
                    break;
                }
            }

            if (!inserted)
            {
                newOrder.id = currentExtensionId++;
                fseek(file, 0, SEEK_END);
                fwrite(&newOrder, sizeof(Order), 1, file);
                inserted = 1;
            }
            break;
        }

        previous = current;
        previousPos = currentPos;
        currentPos = ftell(file);
    }

    if (!inserted) 
	{
        fseek(file, 0, SEEK_END);
        fwrite(&newOrder, sizeof(Order), 1, file);
        printf("Pedido ID %lld inserido no final.\n", order.id);
    }

    fclose(file);
    return 1;
}

int insertProduct(const Product product)
{
    FILE *dataFile = fopen(BIN_PRODUCT, "r+b");
    FILE *indexFile = fopen(INDEX_PRODUCT, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    Product newProduct = product;
    newProduct.active = '1';
    newProduct.next = -1;

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

        if (ie.key <= product.id)
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
        blockOffset = ie.segmentBase;
    }

    fseek(dataFile, blockOffset, SEEK_SET);
    Product current, previous;
    long currentPos = ftell(dataFile);
    long previousPos = -1;
    int inserted = 0;

    while (fread(&current, sizeof(Product), 1, dataFile))
    {
        if (current.active == '0')
        {
            previousPos = currentPos;
            currentPos = ftell(dataFile);
            continue;
        }

        if (current.id > newProduct.id)
        {
            newProduct.id = currentExtensionId++;
            fseek(dataFile, 0, SEEK_END);
            long newOffset = ftell(dataFile);
            fwrite(&newProduct, sizeof(Product), 1, dataFile);

            if (previousPos != -1)
            {
                fseek(dataFile, previousPos, SEEK_SET);
                fread(&previous, sizeof(Product), 1, dataFile);
                previous.next = newProduct.id;
                fseek(dataFile, previousPos, SEEK_SET);
                fwrite(&previous, sizeof(Product), 1, dataFile);
            }

            inserted = 1;
            break;
        }

        previousPos = currentPos;
        currentPos = ftell(dataFile);
    }

    if (!inserted)
    {
        fseek(dataFile, 0, SEEK_END);
        fwrite(&newProduct, sizeof(Product), 1, dataFile);
        printf("Produto ID %lld inserido no final.\n", newProduct.id);
    }

    fclose(dataFile);
    fclose(indexFile);

    return 1;
}

//Todo: insert with extension only
/*
int insertOrder(const Order order)
{
    FILE *file = fopen(BIN_ORDER, "ab");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    fwrite(&order, sizeof(Order), 1, file);
    fclose(file);

    return 1;
}

int insertProduct(const Product product)
{
    FILE *file = fopen(BIN_PRODUCT, "ab");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    fwrite(&product, sizeof(Product), 1, file);
    fclose(file);

    return 1;
}
*/

int removeProduct(const ll productId)
{
    FILE *file = fopen(BIN_PRODUCT, "r+b");
    if (!file)
	{
        printf("Erro ao abrir arquivo de produtos.\n");
        return 0;
    }

    Product p;
    int found = 0;
    long position = 0;

    while (fread(&p, sizeof(Product), 1, file))
	{
        if (p.id == productId && p.active != '0') 
		{
            p.active = '0';
            fseek(file, position, SEEK_SET);
            fwrite(&p, sizeof(Product), 1, file);
            found = 1;
            break;
        }
        position = ftell(file);
    }

    fclose(file);

    if (found)
	{
        printf("Produto ID %lld marcado como excluido.\n", productId);
        return 1;
    }
	else
	{
        printf("Produto ID %lld nao encontrado ou ja excluido.\n", productId);
        return 0;
    }
}

int removeOrder(const ll orderId)
{
    FILE *file = fopen(BIN_ORDER, "r+b");
    if (!file)
	{
        printf("Erro ao abrir arquivo de pedidos.\n");
        return 0;
    }

    Order o;
    int found = 0;
    long position = 0;

    while (fread(&o, sizeof(Order), 1, file))
	{
        if (o.id == orderId && o.active != '0')
		{
            o.active = '0';
            fseek(file, position, SEEK_SET);
            fwrite(&o, sizeof(Order), 1, file);
            found = 1;
            break;
        }
        position = ftell(file);
    }

    fclose(file);

    if (found)
	{
        printf("Pedido ID %lld marcado como excluido.\n", orderId);
        return 1;
    }
	else
	{
        printf("Pedido ID %lld nao encontrado ou ja excluido.\n", orderId);
        return 0;
    }
}

Order createNewOrder()
{
    Order newOrder;

    printf("Inserir novo pedido:\n");
    printf("ID do pedido: ");
    scanf("%lld", &newOrder.id);
    printf("ID do produto: ");
    scanf("%lld", &newOrder.purchasedProductId);
    printf("ID do usuario: ");
    scanf("%lld", &newOrder.userId);
    printf("Quantidade: ");
    scanf("%d", &newOrder.skuQty);

    newOrder.dateTime = time(NULL);
    newOrder.active = '1';

    return newOrder;
}

Product createNewProduct()
{
    Product newProduct;

    printf("Inserir novo produto:\n");
    printf("ID do produto: ");
    scanf("%lld", &newProduct.id);
    printf("ID da categoria: ");
    scanf("%lld", &newProduct.categoryId);
    printf("Alias da categoria: ");
    scanf("%s", newProduct.categoryAlias);
    printf("ID da marca: ");
    scanf("%lld", &newProduct.brandId);
    printf("Preco (ex: 29.99): ");
    float price;
    scanf("%f", &price);
    newProduct.price = (int)(price * 100);
    printf("Genero (M/F/U): ");
    scanf(" %c", &newProduct.productGender);

    newProduct.active = '1';

    return newProduct;
}
