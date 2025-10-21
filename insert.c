/*
int insertOrder(const Order order);
int insertProduct(const Product product);
int removeProduct(const ll productId);
int removeOrder(const ll orderId);
int insertOrderWithExtension(const Order order);
*/

int insertOrderWithExtension(const Order order)
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

//Todo: insert with extension only
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
