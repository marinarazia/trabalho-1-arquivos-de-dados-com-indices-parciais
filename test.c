
void benchmark()
{
    ll testUsers[] = {1515915625216376356, 1515915625150215144, 1515915625118578609};
    ll testOrders[] = {1924719191579951782, 1927543639139943380, 1925511016616034733};
    ll testProducts[] = {1806829193678291446, 1836524717922582619, 1807251125804466651};

    int numTests = 3;

    // acumuladores de tempo
    double totalScan = 0;
    double totalHash = 0;
    double totalProdId = 0;
    double totalOrderId = 0;
    double totalInsertProd = 0;
    double totalInsertOrder = 0;
    double totalRemoveProd = 0;
    double totalRemoveOrder = 0;
    double totalIndexProduct = 0;
    double totalIndexOrder = 0;

    for (int j = 0; j < 1; ++j)
    {
        for (int i = 0; i < numTests; ++i)
        {
            clock_t start = clock();
            int foundScan = searchOrdersByUser(testUsers[i]);
            clock_t end = clock();
            double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalScan += elapsed;

            // Busca com HASH
            start = clock();
            int foundHash = searchOrdersByUserHash(hashTable, testUsers[i]);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalHash += elapsed;

            // Product ID
            start = clock();
            searchProductById(testProducts[i]);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalProdId += elapsed;

            // Order ID
            start = clock();
            searchOrderById(testOrders[i]);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalOrderId += elapsed;

            // Inserts
            Product* newProduct = malloc(sizeof(Product));
            *newProduct = (Product){
                .id = 1,
                .active = '1',
                .next = -1,
                .categoryId = 1,
                .brandId = 1,
                .price = 100,            // 1.00
                .productGender = 'M',
                .categoryAlias = {0}
            };

            Order* newOrder = malloc(sizeof(Order));
            *newOrder = (Order){
                .id = 2,
                .active = '1',
                .next = -1,
                .userId = 1,
                .purchasedProductId = 1,
                .dateTime = time(NULL),
                .skuQty = 1
            };

            start = clock();
            insert(PRODUCT_DAT, PRODUCT_INDEX, newProduct, sizeof(Product), &productTree);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalInsertProd += elapsed;
            status.modificationsProduct++;

            // Remoções
            start = clock();
            removeProduct(newProduct->id);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalRemoveProd += elapsed;

            start = clock();
            insert(ORDER_DAT, ORDER_INDEX, newOrder, sizeof(Order), &orderTree);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalInsertOrder += elapsed;
            status.modificationsOrder++;

            start = clock();
            removeOrder(newOrder->id);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalRemoveOrder += elapsed;

            start = clock();
            createIndex(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product), &productTree);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalIndexProduct += elapsed;

            start = clock();
            createIndex(ORDER_DAT, ORDER_INDEX, sizeof(Order), &orderTree);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalIndexOrder += elapsed;

            free(newOrder);
            free(newProduct);
        }
    }

    // ---- RESUMO FINAL ----
    printf("\n===== TEMPOS TOTAIS =====\n");
    printf("Total SCAN: %.4f s\n", totalScan);
    printf("Total HASH: %.4f s\n", totalHash);
    printf("Total Busca Product ID: %.4f s\n", totalProdId);
    printf("Total Busca Order ID: %.4f s\n", totalOrderId);
    printf("Total Insert Product: %.4f s\n", totalInsertProd);
    printf("Total Insert Order: %.4f s\n", totalInsertOrder);
    printf("Total Remove Product: %.4f s\n", totalRemoveProd);
    printf("Total Remove Order: %.4f s\n", totalRemoveOrder);
    printf("Total Index Product: %.4f s\n", totalIndexProduct);
    printf("Total Index Order: %.4f s\n", totalIndexOrder);
}
