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

    printf("Tentando inserir pedido ID: %lld\n", order.id);

    Order existing, lastInChain;
    long lastPos = -1;
    int idExists = 0;
    
    fseek(file, 0, SEEK_SET);
    while (fread(&existing, sizeof(Order), 1, file)) 
	{
        if (existing.active != '0' && existing.id == order.id) 
		{
            idExists = 1;
            lastInChain = existing;
            lastPos = ftell(file) - sizeof(Order);
            printf("Encontrado ID existente: %lld na posicao %ld\n", existing.id, lastPos);
            
            while (lastInChain.next != -1)
			{
                printf("Seguindo elo: %lld -> %lld\n", lastInChain.id, lastInChain.next);

                fseek(file, 0, SEEK_SET);
                int foundNext = 0;
                Order nextOrder;
                while (fread(&nextOrder, sizeof(Order), 1, file)) 
				{
                    if (nextOrder.active != '0' && nextOrder.id == lastInChain.next) 
					{
                        lastInChain = nextOrder;
                        lastPos = ftell(file) - sizeof(Order);
                        foundNext = 1;
                        printf("Encontrado proximo elo: %lld\n", lastInChain.id);
                        break;
                    }
                }
                
                if (!foundNext) {
                    printf("Elo %lld nao encontrado, parando cadeia.\n", lastInChain.next);
                    break;
                }
            }
        }
    }

    if (idExists) 
	{
        printf("ID %lld ja existe. ", order.id);
        printf("Ultimo da cadeia: ID %lld (next=%lld)\n", lastInChain.id, lastInChain.next);
        
        Order extensionOrder = newOrder;
        extensionOrder.id = currentExtensionId++; 
        extensionOrder.next = -1; 
        
        fseek(file, 0, SEEK_END);
        long extensionPos = ftell(file);
        fwrite(&extensionOrder, sizeof(Order), 1, file);
        printf("Novo registro de extensao criado: ID %lld\n", extensionOrder.id);
        
        lastInChain.next = extensionOrder.id;
        fseek(file, lastPos, SEEK_SET);
        fwrite(&lastInChain, sizeof(Order), 1, file);
        
        printf("Elo criado: %lld -> %lld\n", lastInChain.id, extensionOrder.id);
    } 
	else 
	{
        fseek(file, 0, SEEK_END);
        fwrite(&newOrder, sizeof(Order), 1, file);
        printf("Pedido ID %lld inserido normalmente (sem elo).\n", order.id);
    }

    fclose(file);
    return 1;
}

int insertProduct(const Product product)
{
    FILE *file = fopen(BIN_PRODUCT, "r+b");
    if (!file) return 0;

    Product newProduct = product;
    newProduct.active = '1';
    newProduct.next = -1;

    printf("Inserindo produto ID: %lld\n", product.id);

    Product existing;
    int idExists = 0;
    long existingPos = -1;
    
    fseek(file, 0, SEEK_SET);
    while (fread(&existing, sizeof(Product), 1, file)) 
	{
        if (existing.active != '0' && existing.id == product.id) 
		{
            idExists = 1;
            existingPos = ftell(file) - sizeof(Product);
            break;
        }
    }

    if (idExists) 
	{
        printf("ID %lld ja existe.", product.id);
        
        Product extensionProduct = newProduct;
        extensionProduct.id = currentExtensionId++; 
        extensionProduct.next = -1;
        
        fseek(file, 0, SEEK_END);
        long newPos = ftell(file);
        fwrite(&extensionProduct, sizeof(Product), 1, file);
        
        existing.next = extensionProduct.id;
        fseek(file, existingPos, SEEK_SET);
        fwrite(&existing, sizeof(Product), 1, file);
        
        printf("Elo criado: %lld -> %lld\n", existing.id, extensionProduct.id);
        
    } 
	else 
	{
        fseek(file, 0, SEEK_END);
        fwrite(&newProduct, sizeof(Product), 1, file);
        printf("Produto ID %lld inserido normalmente.\n", product.id);
    }

    fclose(file);
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
	float price;
	
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
    scanf("%f", &price);
    newProduct.price = (int)(price * 100);
    getchar();
    printf("Genero (M/F/U): ");
    scanf(" %c", &newProduct.productGender);

    newProduct.active = '1';
    newProduct.next = -1;

    return newProduct;
}
