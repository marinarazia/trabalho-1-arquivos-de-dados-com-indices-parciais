/*
int insertOrder(const Order order);
int insertProduct(const Product product);
int removeProduct(const ll productId);
int removeOrder(const ll orderId);
*/

int insertOrder(Order order)
{
    return 0;
}

int insertProduct(Product product)
{
    FILE *dataFile = fopen(BIN_PRODUCT, "r+b");
    FILE *indexFile = fopen(INDEX_PRODUCT, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    fseekBlockOffset(dataFile, indexFile, product.id);

    Product current, previous;
    long currentPos = ftell(dataFile);
    long previousPos = -1;
    int inserted = 0;

    /*
    fread(&current, sizeof(Product), 1, dataFile)
    if (current.id < product.id)
    {
        insert at start of file
    }
    fseek(datafile, 0, SEEK_START);
    */

    while (fread(&current, sizeof(Product), 1, dataFile))
    {
        if (current.active == '0')
        {
            currentPos = ftell(dataFile);
            continue;
        }

        if (current.id >= product.id)
        {
            while (current.next)
            {
                previousPos = currentPos;
                fseek(dataFile, current.next, SEEK_SET);
                currentPos = ftell(dataFile);
                fread(&current, sizeof(Product), 1, dataFile);
            }

            fseek(dataFile, 0, SEEK_END);
            long offset = ftell(dataFile);
            product.next = current.next;
            fwrite(&product, sizeof(Product), 1, dataFile);

            if (previousPos != -1)
            {
                fseek(dataFile, previousPos, SEEK_SET);
                fread(&previous, sizeof(Product), 1, dataFile);
                previous.next = offset;
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
        long offset = ftell(dataFile);
        fwrite(&product, sizeof(Product), 1, dataFile);
        printf("Produto ID %lld inserido no final.\n", product.id);

        if (previousPos != -1)
        {
            fseek(dataFile, previousPos, SEEK_SET);
            fread(&previous, sizeof(Product), 1, dataFile);
            previous.next = offset;
            fseek(dataFile, previousPos, SEEK_SET);
            fwrite(&previous, sizeof(Product), 1, dataFile);
        }
    }

    fclose(dataFile);
    fclose(indexFile);
    return 1;
}

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
    newOrder.next = -1;

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

    newProduct.next = -1;
    newProduct.active = '1';

    return newProduct;
}
