/*
int createIndex();
void convertTextToBinary();
*/

//Todo: Ricardo implementar
int restructureDataset()
{
    return 0;
}

int createIndex() 
{
    FILE *orders = fopen(BIN_ORDER, "rb");
    FILE *products = fopen(BIN_PRODUCT, "rb");
    FILE *indexOrders = fopen(INDEX_ORDER, "wb");
    FILE *indexProducts = fopen(INDEX_PRODUCT, "wb");

    if (!orders || !products || !indexOrders || !indexProducts)
    {
        printf("Erro ao abrir arquivos para indexacao.\n");
        return 0;
    }

    Order o;
    Product p;
    long pos = 0;
    int count = 0;

    while (fread(&o, sizeof(Order), 1, orders))
    {
        if (count % SEGMENT_SIZE == 0) 
		{
            Index idx = {o.id, pos};
            fwrite(&idx, sizeof(Index), 1, indexOrders);
        }
        pos = ftell(orders);
        count++;
    }

    pos = 0;
    count = 0;

    while (fread(&p, sizeof(Product), 1, products)) 
	{
        if (count % SEGMENT_SIZE == 0) 
		{
            Index idx = {p.purchasedProductId, pos};
            fwrite(&idx, sizeof(Index), 1, indexProducts);
        }
        pos = ftell(products);
        count++;
    }

    fclose(orders);
    fclose(products);
    fclose(indexOrders);
    fclose(indexProducts);

    printf("Indices criados.\n");
    return 1;
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

        strncpy(dateStr, cols[0], sizeof(dateStr)-1);
        o.id = strlen(cols[1]) ? atoll(cols[1]) : -1;
        o.purchasedProductId = strlen(cols[2]) ? atoll(cols[2]) : -1;
        o.skuQty = strlen(cols[3]) ? atoi(cols[3]) : -1;
        p.categoryId = strlen(cols[4]) ? atoll(cols[4]) : -1;
        strncpy(p.categoryAlias, cols[5], MAX_CATEGORY_ALIAS-1);
        p.brandId = strlen(cols[6]) ? atoll(cols[6]) : -1;
        float priceFloat = strlen(cols[7]) ? atof(cols[7]) : 0.0f;
        p.price = (int)(priceFloat * 10 * USD_DECIMAL + 0.5f);
        o.userId = strlen(cols[8]) ? atoll(cols[8]) : -1;
        p.productGender = strlen(cols[9]) ? cols[9][0] : ' ';

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

        p.purchasedProductId = o.purchasedProductId;

        fwrite(&o, sizeof(Order), 1, orders);
        fwrite(&p, sizeof(Product), 1, products);
    }

    fclose(csv);
    fclose(orders);
    fclose(products);
    printf("Conversao concluida.\n");
}

