
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

    for (int j = 0; j < 50; ++j)
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
            Product newProduct = { 1, '1', 1, 1, 1, 1, '0', 1};
            Order newOrder = { 2, '1', 1, 1, 1, time(NULL), 1};

            start = clock();
            insert(PRODUCT_DAT, PRODUCT_INDEX, &newProduct, sizeof(Product), &productTree);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalInsertProd += elapsed;

            start = clock();
            insert(ORDER_DAT, ORDER_INDEX, &newOrder, sizeof(Order), &orderTree);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalInsertOrder += elapsed;

            // Remoções
            start = clock();
            removeProduct(newProduct.id);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalRemoveProd += elapsed;

            start = clock();
            removeOrder(newOrder.id);
            end = clock();
            elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            totalRemoveOrder += elapsed;
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
}
