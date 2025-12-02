/*
int insert(const char* dataFileName, 
           const char* indexFileName, 
           const void* record,
           const size_t recordSize)

int removeProduct(const ll productId);
int removeOrder(const ll orderId);
*/

int insert(const char* dataFileName, 
           const char* indexFileName, 
           const void* record,
           const size_t recordSize,
           BPTree* btree)
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

    // se head lógico vazio e primeiro record fisíco > record sendo inserido
    // offset head lógico = offset record sendo inserido
    fseek(dataFile, 0, SEEK_SET);
    fread(currBuffer, recordSize, 1, dataFile);
    RecordHeader *curr = (RecordHeader*)currBuffer;
    if (*headPtr == -1 && rec->id < curr->id)
    {
        *headPtr = newPos;
        fseek(statusFile, 0, SEEK_SET);
        fwrite(&status, sizeof(Status), 1, statusFile);
        printf("File empty. New head at %ld\n", newPos);
        goto cleanup;
    }
    
    // caso head lógico exista (registro menor que primeiro registro físico)
    int insertAfterHead = 0;
    if (*headPtr != -1)
    {
        // inserir antes de head se necessario
        fseek(dataFile, *headPtr, SEEK_SET);
        fread(currBuffer, recordSize, 1, dataFile);
        curr = (RecordHeader*)currBuffer;
        if (rec->id < curr->id)
        {
            rec->next = *headPtr;
            fseek(dataFile, newPos, SEEK_SET);
            fwrite(rec, recordSize, 1, dataFile);
            *headPtr = newPos;
            fseek(statusFile, 0, SEEK_SET);
            fwrite(&status, sizeof(Status), 1, statusFile);
            printf("Inserted new head record ID %lld\n", rec->id);
            goto cleanup;
        }
        // inserir depois de head lógico
        rewind(dataFile);
        fread(currBuffer, recordSize, 1, dataFile);
        curr = (RecordHeader*)currBuffer;
        if (rec->id < curr->id)
        {
            insertAfterHead = 1;
        }
    }

    fseek(dataFile, *headPtr, SEEK_SET); 
    if (!insertAfterHead)
    {
#if USE_BTREE_INDEX
    fseekBTreeOffset(dataFile, btree, rec->id);
#else
    fseekSegmentOffset(dataFile, indexFile, rec->id);
#endif
    }

    //inserir no meio
    long prevPos = -1;
    while (fread(currBuffer, recordSize, 1, dataFile))
    {
        curr = (RecordHeader*)currBuffer;

        if (curr->id > rec->id)
        {
            prevPos = ftell(dataFile) - (recordSize * 2);
            fseek(dataFile, prevPos, SEEK_SET);
            fread(prevBuffer, recordSize, 1, dataFile);
            RecordHeader *prev = (RecordHeader*)prevBuffer;

            long nextPos = prev->next;
            long lastValidPos = prevPos;

            while (nextPos != -1)
            {
                fseek(dataFile, nextPos, SEEK_SET);
                fread(currBuffer, recordSize, 1, dataFile);
                RecordHeader *nextRec = (RecordHeader*)currBuffer;

                if (nextRec->id > rec->id)
                    break;

                lastValidPos = nextPos;
                nextPos = nextRec->next;
            }

            fseek(dataFile, lastValidPos, SEEK_SET);
            fread(prevBuffer, recordSize, 1, dataFile);
            prev = (RecordHeader*)prevBuffer;

            rec->next = prev->next;
            prev->next = newPos;

            fseek(dataFile, lastValidPos, SEEK_SET);
            fwrite(prev, recordSize, 1, dataFile);

            fseek(dataFile, newPos, SEEK_SET);
            fwrite(rec, recordSize, 1, dataFile);

            printf("Inserted record ID %lld after ID %lld\n", rec->id, prev->id);
            goto cleanup;
        }
    }

    // ligar final extensão de head lógica
    if (insertAfterHead)
    {
        long currPos = *headPtr;
        unsigned char *buffer = malloc(recordSize);
        if (!buffer)
            goto cleanup;

        RecordHeader *curr = (RecordHeader*)buffer;

        while (1)
        {
            fseek(dataFile, currPos, SEEK_SET);
            fread(buffer, recordSize, 1, dataFile);

            if (curr->next == -1)
                break;

            currPos = curr->next;
        }

        curr->next = newPos;

        fseek(dataFile, currPos, SEEK_SET);
        fwrite(buffer, recordSize, 1, dataFile);

        fseek(dataFile, newPos, SEEK_SET);
        fwrite(rec, recordSize, 1, dataFile);

        printf("Appended record ID %lld after ID %lld (end of head chain)\n",
               rec->id, curr->id);

        free(buffer);
    }

cleanup:
    fclose(dataFile);
    fclose(indexFile);
    fclose(statusFile);
    free(currBuffer);
    free(prevBuffer);

    return 1;
}

//Todo: generalize remove function
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

#if USE_BTREE_INDEX
    fseekBTreeOffset(dataFile, &productTree, productId);
#else
    fseekSegmentOffset(dataFile, indexFile, productId);
#endif

    Product p;
    int found = 0;
    long position = ftell(dataFile);
    int checked = 0;
    while (fread(&p, sizeof(Product), 1, dataFile) && checked < SEGMENT_SIZE)
	{
        checked++;

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

    printf("Produto ID %lld nao encontrado ou ja excluido.\n", productId);
    return 0;
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

#if USE_BTREE_INDEX
    fseekBTreeOffset(dataFile, &orderTree, orderId);
#else
    fseekSegmentOffset(dataFile, indexFile, orderId);
#endif

    Order o;
    int found = 0;
    long position = ftell(dataFile);
    int checked = 0;
    while (fread(&o, sizeof(Order), 1, dataFile) && checked < SEGMENT_SIZE)
	{
        checked++;

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

    printf("Pedido ID %lld nao encontrado ou ja excluido.\n", orderId);
    return 0;
}
