/*
int createIndex(const char *dataFile, const char *indexFile, size_t recordSize)
void reorganizeProductFile()
void reorganizeOrderFile()
void convertTextToBinary()
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

//Todo: generalize the reorganize functions
void reorganizeProductFile()
{
    printf("Reorganizando arquivo de produtos...\n");

    FILE *oldFile = fopen(PRODUCT_DAT, "rb");
    FILE *newFile = fopen("temp_reorg.bin", "wb");
    if (!oldFile || !newFile)
    {
        printf("Erro ao abrir arquivos.\n");
        if (oldFile) fclose(oldFile);
        if (newFile) fclose(newFile);
        return;
    }

    // insert head and its extensions
    if (status.headProduct != -1)
    {
        long nextOffset = status.headProduct;
        while (nextOffset != -1)
        {
            Product p;
            fseek(oldFile, nextOffset, SEEK_SET);
            fread(&p, sizeof(Product), 1, oldFile);

            long tmpNext = p.next;
            p.next = -1; // reset next
            fwrite(&p, sizeof(Product), 1, newFile);

            nextOffset = tmpNext;
        }
    }

    // scan old file sequentially inserting records and its extensions until extension area
    fseek(oldFile, 0, SEEK_END);
    long totalRecords = ftell(oldFile) / sizeof(Product);
    rewind(oldFile);

    ll lastId = -1;
    for (long i = 0; i < totalRecords; i++)
    {
        long offset = i * sizeof(Product);

        Product p;
        fseek(oldFile, offset, SEEK_SET);
        fread(&p, sizeof(Product), 1, oldFile);

        if (p.id < lastId) break;
        lastId = p.id;

        if (p.active == '0') continue;

        long nextOffset = p.next;

        p.next = -1;
        fwrite(&p, sizeof(Product), 1, newFile);

        while (nextOffset != -1)
        {
            Product nextP;
            fseek(oldFile, nextOffset, SEEK_SET);
            fread(&nextP, sizeof(Product), 1, oldFile);

            if (nextP.active == '0') break;

            long tmpNext = nextP.next;
            nextP.next = -1;
            fwrite(&nextP, sizeof(Product), 1, newFile);

            nextOffset = tmpNext;
        }
    }

    fclose(oldFile);
    fclose(newFile);

    remove(PRODUCT_DAT);
    rename("temp_products_reorg.bin", PRODUCT_DAT);

    status.modificationsProduct = 0;
    status.headProduct = -1;
    FILE *statusFile = fopen(STATUS_DAT, "wb");
    if (statusFile)
    {
        fwrite(&status, sizeof(Status), 1, statusFile);
        fclose(statusFile);
    }

    createIndex(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product));
    printf("Reorganização concluída com sucesso.\n");
}

void reorganizeOrderFile()
{
    printf("Reorganizando arquivo de pedidos...\n");

    FILE *oldFile = fopen(ORDER_DAT, "rb");
    FILE *newFile = fopen("temp_orders_reorg.bin", "wb");
    if (!oldFile || !newFile)
    {
        printf("Erro ao abrir arquivos.\n");
        if (oldFile) fclose(oldFile);
        if (newFile) fclose(newFile);
        return;
    }

    if (status.headOrder != -1)
    {
        long nextOffset = status.headOrder;
        while (nextOffset != -1)
        {
            Order o;
            fseek(oldFile, nextOffset, SEEK_SET);
            fread(&o, sizeof(Order), 1, oldFile);

            long tmpNext = o.next;
            o.next = -1; // reseta o ponteiro
            fwrite(&o, sizeof(Order), 1, newFile);

            nextOffset = tmpNext;
        }
    }

    fseek(oldFile, 0, SEEK_END);
    long totalRecords = ftell(oldFile) / sizeof(Order);
    rewind(oldFile);

    ll lastId = -1;
    for (long i = 0; i < totalRecords; i++)
    {
        long offset = i * sizeof(Order);

        Order o;
        fseek(oldFile, offset, SEEK_SET);
        fread(&o, sizeof(Order), 1, oldFile);

        if (o.id < lastId) break; 
        lastId = o.id;

        if (o.active == '0') continue;

        long nextOffset = o.next;

        o.next = -1;
        fwrite(&o, sizeof(Order), 1, newFile);

        while (nextOffset != -1)
        {
            Order nextO;
            fseek(oldFile, nextOffset, SEEK_SET);
            fread(&nextO, sizeof(Order), 1, oldFile);

            if (nextO.active == '0') break;

            long tmpNext = nextO.next;
            nextO.next = -1;
            fwrite(&nextO, sizeof(Order), 1, newFile);

            nextOffset = tmpNext;
        }
    }

    fclose(oldFile);
    fclose(newFile);

    remove(ORDER_DAT);
    rename("temp_orders_reorg.bin", ORDER_DAT);

    status.modificationsOrder = 0;
    status.headOrder = -1;

    FILE *statusFile = fopen(STATUS_DAT, "wb");
    if (statusFile)
    {
        fwrite(&status, sizeof(Status), 1, statusFile);
        fclose(statusFile);
    }

    createIndex(ORDER_DAT, ORDER_INDEX, sizeof(Order));
    printf("Reorganização de pedidos concluída com sucesso.\n");
}

void convertTextToBinary() 
{
    FILE *csv = fopen(DATASET_CSV, "r");
    FILE *orders = fopen(ORDER_DAT, "wb");
    FILE *products = fopen(PRODUCT_DAT, "wb");

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
