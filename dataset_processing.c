/*
int createIndex(const char *dataFile, const char *indexFile, size_t recordSize)
void convertTextToBinary()
void reorganizeOrderFile()
void reorganizeProductFile()
*/

int createIndex(const char *dataFile, 
                const char *indexFile, 
                size_t recordSize)
{
    FILE *data = fopen(dataFile, "rb");
    FILE *index = fopen(indexFile, "wb");

    if (!data || !index)
    {
        printf("Erro ao abrir arquivos: %s ou %s\n", dataFile, indexFile);
        if (data) fclose(data);
        if (index) fclose(index);
        return 0;
    }

    void *record = malloc(recordSize);
    if (!record)
    {
        printf("Erro de memoria ao criar indice.\n");
        fclose(data);
        fclose(index);
        return 0;
    }

    long segmentStart = 0;
    int count = 0;

    ll lastId = -1;

    while (fread(record, recordSize, 1, data) == 1)
    {
        ll id = *(ll *)record;
        lastId = id;
        count++;

        if (count % SEGMENT_SIZE == 0)
        {
            Index idx = { lastId, segmentStart };
            fwrite(&idx, sizeof(Index), 1, index);

            segmentStart = ftell(data);
        }
    }

    if (count % SEGMENT_SIZE != 0)
    {
        Index idx = { lastId, segmentStart };
        fwrite(&idx, sizeof(Index), 1, index);
    }

    free(record);
    fclose(data);
    fclose(index);

    printf("Indice criado para %s com %d registros.\n", dataFile, count);
    return 1;
}

void reorganizeProductFile() 
{
    printf("Reorganizando arquivo de produtos...\n");

    FILE *oldFile = fopen(BIN_PRODUCT, "rb");
    FILE *newFile = fopen("temp_products_reorg.bin", "wb");
    if (!oldFile || !newFile) 
    {
        printf("Erro ao reorganizar arquivo.\n");
        if (oldFile) fclose(oldFile);
        if (newFile) fclose(newFile);
        return;
    }

    Product p;
    while (fread(&p, sizeof(Product), 1, oldFile))
    {
        if (p.active == '0') continue;

        long next = p.next;
        p.next = -1;
        fwrite(&p, sizeof(Product), 1, newFile);

        while (next != -1)
        {
            fseek(oldFile, next, SEEK_SET);
            fread(&p, sizeof(Product), 1, oldFile);
            next = p.next;
            p.next = -1;
            if (p.active == '0') continue;
            fwrite(&p, sizeof(Product), 1, newFile);
        }
    }

    fclose(oldFile);
    fclose(newFile);

    remove(BIN_PRODUCT);
    rename("temp_products_reorg.bin", BIN_PRODUCT);

    status.currentExtensionId = EXTENSION_AREA_START;
    status.modificationsProduct = 0;
    FILE *statusFile = fopen(BIN_STATUS, "wb");
    if (statusFile)
    {
        fwrite(&status, sizeof(Status), 1, statusFile);
        fclose(statusFile);
    }

    createIndex(BIN_PRODUCT, INDEX_PRODUCT, sizeof(Product));
}

void reorganizeOrderFile() 
{
    printf("Reorganizando arquivo de pedidos...\n");

    FILE *oldFile = fopen(BIN_ORDER, "rb");
    FILE *newFile = fopen("temp_orders_reorg.bin", "wb");

    if (!oldFile || !newFile) 
    {
        printf("Erro ao reorganizar arquivo.\n");
        if (oldFile) fclose(oldFile);
        if (newFile) fclose(newFile);
        return;
    }

    Order o;
    while (fread(&o, sizeof(Order), 1, oldFile))
    {
        if (o.active == '0') continue;

        long next = o.next;
        o.next = -1;
        fwrite(&o, sizeof(Order), 1, newFile);

        while (next != -1)
        {
            fseek(oldFile, next, SEEK_SET);
            fread(&o, sizeof(Order), 1, oldFile);

            next = o.next;
            o.next = -1;

            if (o.active == '0') continue;

            fwrite(&o, sizeof(Order), 1, newFile);
        }
    }

    fclose(oldFile);
    fclose(newFile);

    remove(BIN_ORDER);
    rename("temp_orders_reorg.bin", BIN_ORDER);

    status.currentExtensionId = EXTENSION_AREA_START;
    status.modificationsOrder = 0;
    FILE *statusFile = fopen(BIN_STATUS, "wb");
    if (statusFile)
    {
        fwrite(&status, sizeof(Status), 1, statusFile);
        fclose(statusFile);
    }

    createIndex(BIN_ORDER, INDEX_ORDER, sizeof(Order));
}

void convertTextToBinary() 
{
    FILE *csv = fopen(DATASET_CSV, "r");
    FILE *orders = fopen(BIN_ORDER, "wb");
    FILE *products = fopen(BIN_PRODUCT, "wb");

    if (!csv || !orders || !products) 
	{
        printf("Erro ao abrir arquivos.\n");
        exit(1);
    }

    char line[512];
    fgets(line, sizeof(line), csv);

    while (fgets(line, sizeof(line), csv)) 
	{
        line[strcspn(line, "\r\n")] = 0;

        Order o = {0};
        Product p = {0};
        char dateStr[32] = {0};
        char cols[10][64] = {{0}};

        int col = 0;
        char *start = line;
        char *end;
        while (col < 10) 
		{
            end = strchr(start, ',');
            if (end) 
			{
                size_t len = end - start;
                if (len >= sizeof(cols[col])) len = sizeof(cols[col])-1;
                strncpy(cols[col], start, len);
                cols[col][len] = '\0';
                start = end + 1;
            } 
			else 
			{
                strncpy(cols[col], start, sizeof(cols[col])-1);
                cols[col][sizeof(cols[col])-1] = '\0';
            }
            col++;
            if (!end) break;
        }

        //order
        strncpy(dateStr, cols[0], sizeof(dateStr)-1);
        o.id = strlen(cols[1]) ? atoll(cols[1]) : -1;
        o.purchasedProductId = strlen(cols[2]) ? atoll(cols[2]) : -1;
        o.skuQty = strlen(cols[3]) ? atoi(cols[3]) : -1;
        o.userId = strlen(cols[8]) ? atoll(cols[8]) : -1;
        o.next = -1;

        //product
        p.categoryId = strlen(cols[4]) ? atoll(cols[4]) : -1;
        strncpy(p.categoryAlias, cols[5], MAX_CATEGORY_ALIAS-1);
        p.brandId = strlen(cols[6]) ? atoll(cols[6]) : -1;
        float priceFloat = strlen(cols[7]) ? atof(cols[7]) : 0.0f;
        p.price = (int)(priceFloat * 100);
        p.productGender = strlen(cols[9]) ? cols[9][0] : ' ';
        p.id = o.purchasedProductId;
        p.next = -1;

        struct tm tm = {0};
        int y,m,d,H,M,S;
        o.dateTime = 0;
        
        if (sscanf(dateStr, "%d-%d-%d %d:%d:%d", &y,&m,&d,&H,&M,&S) == 6) 
		{
            tm.tm_year = y - 1900;
            tm.tm_mon = m - 1;
            tm.tm_mday = d;
            tm.tm_hour = H;
            tm.tm_min = M;
            tm.tm_sec = S;
            o.dateTime = mktime(&tm);
        }

        fwrite(&o, sizeof(Order), 1, orders);
        fwrite(&p, sizeof(Product), 1, products);
    }

    fclose(csv);
    fclose(orders);
    fclose(products);
    printf("Conversao concluida.\n");
}
