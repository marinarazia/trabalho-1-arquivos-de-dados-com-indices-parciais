/*
int insertOrder(const Order order);
int insertProduct(const Product product);
int removeProduct(const ll productId);
int removeOrder(const ll orderId);
*/

int insert(const char* dataFileName, 
           const char* indexFileName, 
           void* record,
           size_t recordSize)
{
    FILE *dataFile = fopen(dataFileName, "r+b");
    FILE *indexFile = fopen(indexFileName, "rb");
    FILE *statusFile = fopen(STATUS_DAT, "r+b");
    if (!dataFile || !indexFile || !statusFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        if (statusFile) fclose(statusFile);
        return 0;
    }

    RecordHeader *rec = (RecordHeader*)record;
    printf("Inserting record ID: %lld\n", rec->id);

    ll *headPtr = getHeadPointer(dataFileName);

    unsigned char *currBuffer = malloc(recordSize);
    unsigned char *prevBuffer = malloc(recordSize);
    if (!currBuffer || !prevBuffer)
    {
        fclose(dataFile);
        fclose(indexFile);
        fclose(statusFile);
        free(currBuffer);
        free(prevBuffer);
        return 0;
    }

    // inserir no final novo registro
    fseek(dataFile, 0, SEEK_END);
    long newPos = ftell(dataFile);
    fwrite(record, recordSize, 1, dataFile);

    // se arquivo vazio novo registro vira head
    if (*headPtr == -1)
    {
        *headPtr = newPos;
        fseek(statusFile, 0, SEEK_SET);
        fwrite(&status, sizeof(Status), 1, statusFile);
        printf("File empty. New head at %ld\n", newPos);
        goto cleanup;
    }

    fseek(dataFile, *headPtr, SEEK_SET);
    fread(currBuffer, recordSize, 1, dataFile);
    RecordHeader *curr = (RecordHeader*)currBuffer;

    // inserir antes de head se necessario
    if (rec->id < curr->id)
    {
        fseek(dataFile, newPos, SEEK_SET);
        fwrite(rec, recordSize, 1, dataFile);

        *headPtr = newPos;
        fseek(statusFile, 0, SEEK_SET);
        fwrite(&status, sizeof(Status), 1, statusFile);
        printf("Inserted new head record ID %lld\n", rec->id);
        goto cleanup;
    }

    fseek(dataFile, *headPtr, SEEK_SET);
    fread(currBuffer, recordSize, 1, dataFile);
    curr = (RecordHeader*)currBuffer;

    //inserir depois de head lógico
    if (rec->id > curr->id && curr->next != -1 && rec->id < ((RecordHeader*)malloc(recordSize))->id)
    {
        rec->next = curr->next;
        curr->next = newPos;

        fseek(dataFile, *headPtr, SEEK_SET);
        fwrite(curr, recordSize, 1, dataFile);

        fseek(dataFile, newPos, SEEK_SET);
        fwrite(rec, recordSize, 1, dataFile);

        printf("Inserted record ID %lld between head ID %lld and its next\n", rec->id, curr->id);
        goto cleanup;
    }

    fseekSegmentOffset(dataFile, indexFile, rec->id);
    long prevPos = -1;

    while (fread(currBuffer, recordSize, 1, dataFile))
    {
        curr = (RecordHeader*)currBuffer;

        if (curr->active == '0') continue;

        // inserir extensão no meio 
        if (curr->id > rec->id)
        {
            prevPos = ftell(dataFile) - (recordSize * 2);
            fseek(dataFile, prevPos, SEEK_SET);
            fread(prevBuffer, recordSize, 1, dataFile);
            RecordHeader *prev = (RecordHeader*)prevBuffer;

            rec->next = prev->next;
            prev->next = newPos;

            fseek(dataFile, prevPos, SEEK_SET);
            fwrite(prev, recordSize, 1, dataFile);

            fseek(dataFile, newPos, SEEK_SET);
            fwrite(rec, recordSize, 1, dataFile);
            printf("Inserted record ID %lld after ID %lld\n", rec->id, prev->id);
            goto cleanup;
        }
    }

cleanup:
    fclose(dataFile);
    fclose(indexFile);
    fclose(statusFile);
    free(currBuffer);
    free(prevBuffer);

    return 1;
}

int removeProduct(const ll productId)
{
    FILE *dataFile = fopen(PRODUCT_DAT, "r+b");
    FILE *indexFile = fopen(PRODUCT_INDEX, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    ll segLastId = fseekSegmentOffset(dataFile, indexFile, productId);

    Product p;
    int found = 0;
    long position = 0;

    while (fread(&p, sizeof(Product), 1, dataFile) && p.id <= segLastId)
	{
        if (p.id == productId && p.active != '0') 
		{
            p.active = '0';
            fseek(dataFile, position, SEEK_SET);
            fwrite(&p, sizeof(Product), 1, dataFile);
            found = 1;
            break;
        }
        position = ftell(dataFile);
    }

    fclose(dataFile);
    fclose(indexFile);

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
    FILE *dataFile = fopen(ORDER_DAT, "r+b");
    FILE *indexFile = fopen(ORDER_INDEX, "rb");
    if (!dataFile || !indexFile)
    {
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return 0;
    }

    ll segLastId = fseekSegmentOffset(dataFile, indexFile, orderId);

    Order o;
    int found = 0;
    long position = 0;

    while (fread(&o, sizeof(Order), 1, dataFile) && o.id <= segLastId)
	{
        if (o.id == orderId && o.active != '0')
		{
            o.active = '0';
            fseek(dataFile, position, SEEK_SET);
            fwrite(&o, sizeof(Order), 1, dataFile);
            found = 1;
            break;
        }
        position = ftell(dataFile);
    }
    

    fclose(dataFile);
    fclose(indexFile);

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
