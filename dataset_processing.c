/*

int createIndex(const char *dataFile, 
                const char *indexFile, 
                const size_t recordSize)

void reorganizeFile(const char* dataFile, 
                    const char* indexFile, 
                    const size_t recordSize)

void convertTextToBinary()

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "entities.h"
#include "config.h"
#include "bpt_index.h"
#include "crypt.h"

int hashFunction(ll key, int tableSize) 
{
    return key % tableSize;
}

HashTable* createHashTable(int size) 
{
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    ht->size = size;
    ht->count = 0;
    ht->table = (HashEntry**)calloc(size, sizeof(HashEntry*));
    return ht;
}

void hashInsert(HashTable *ht, ll key, long offset) 
{
    int index = hashFunction(key, ht->size);
    HashEntry *new_entry = (HashEntry*)malloc(sizeof(HashEntry));
    new_entry->key = key;
    new_entry->file_offset = offset;
    new_entry->next = NULL;
    
    // Encadeamento externo para resolução de colisões
    if (ht->table[index] == NULL) 
    {
        ht->table[index] = new_entry;
    } else 
    {
        HashEntry *current = ht->table[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_entry;
    }
    ht->count++;
}

HashTable* createHashIndex(const char *dataFile, int tableSize) 
{
    FILE *data = fopen(dataFile, "rb");
    if (!data) return NULL;
    
    HashTable *ht = createHashTable(tableSize);
    void *record = malloc(sizeof(Order));
    if (!record) 
    {
        fclose(data);
        free(ht);
        return NULL;
    }
    
    clock_t start = clock();
    
    while (fread(record, sizeof(Order), 1, data) == 1) 
    {
        Order *o = (Order*)record;
        if (o->userId > 0 && o->active != '0') 
        { 
            hashInsert(ht, o->userId, ftell(data) - sizeof(Order));
        }
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Índice Hash criado em %.4f segundos\n", time_spent);
    printf("Tamanho da tabela: %d, Elementos inseridos: %d\n", ht->size, ht->count);
    
    free(record);
    fclose(data);
    return ht;
}

HashEntry* hashSearch(HashTable *ht, ll key) 
{
    if (!ht) return NULL;
    
    int index = hashFunction(key, ht->size);
    HashEntry *current = ht->table[index];
    
    while (current != NULL) 
    {
        if (current->key == key) 
        {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

void freeHashTable(HashTable *ht) 
{
    if (!ht) return;
    
    for (int i = 0; i < ht->size; i++) 
    {
        HashEntry *current = ht->table[i];
        while (current != NULL) 
        {
            HashEntry *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}

int createIndex(const char *dataFile, 
                const char *indexFile, 
                const size_t recordSize, 
                BPTree *bptree)
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

    *bptree = bpt_create();

    // create partial index
    long segmentStart = 0;
    int count = 0;
    ll lastId = -1;
    while (fread(record, recordSize, 1, data) == 1)
    {
        long currentOffset = ftell(data) - recordSize;
        long long id = *(long long*)
        record; lastId = id;
        count++;

        bpt_insert(bptree, id, currentOffset);
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

void reorganizeFile(const char* dataFile, 
                    const char* indexFile, 
                    const size_t recordSize,
                    BPTree *bptree)
{
    printf("Reorganizando arquivo %s...\n", dataFile);

    FILE *oldFile = fopen(dataFile, "rb");
    FILE *newFile = fopen("temp_reorg.bin", "wb");
    if (!oldFile || !newFile)
    {
        printf("Erro ao abrir arquivos.\n");
        if (oldFile) fclose(oldFile);
        if (newFile) fclose(newFile);
        return;
    }

    ll* headOffset = &status.headOrder;
    int* modifications = &status.modificationsOrder;
    if (recordSize == sizeof(Product))
    {
        headOffset = &status.headProduct;
        modifications = &status.modificationsProduct;
    }

    // insert logical head and its extensions
    if (*headOffset != -1)
    {
        long nextOffset = *headOffset;
        void *buffer = malloc(recordSize);
        if (!buffer) { fclose(oldFile); fclose(newFile); return; }

        while (nextOffset != -1)
        {
            fseek(oldFile, nextOffset, SEEK_SET);
            fread(buffer, recordSize, 1, oldFile);

            RecordHeader *hdr = (RecordHeader *)buffer;
            long tmpNext = hdr->next;
            hdr->next = -1;
            fwrite(buffer, recordSize, 1, newFile);

            nextOffset = tmpNext;
        }

        free(buffer);
    }

    // scan dataFile and insert the records and its extensions until extension area
    fseek(oldFile, 0, SEEK_END);
    long totalRecords = ftell(oldFile) / recordSize;
    rewind(oldFile);

    ll lastId = -1;
    void *buffer = malloc(recordSize);
    if (!buffer) { fclose(oldFile); fclose(newFile); return; }

    for (long i = 0; i < totalRecords; i++)
    {
        fseek(oldFile, i * recordSize, SEEK_SET);
        fread(buffer, recordSize, 1, oldFile);

        RecordHeader *hdr = (RecordHeader *)buffer;
        if (hdr->id < lastId) break;
        lastId = hdr->id;

        if (hdr->active == '0') continue;

        long nextOffset = hdr->next;
        hdr->next = -1;
        fwrite(buffer, recordSize, 1, newFile);

        void *nextBuffer = malloc(recordSize);
        if (!nextBuffer) break;

        while (nextOffset != -1)
        {
            fseek(oldFile, nextOffset, SEEK_SET);
            fread(nextBuffer, recordSize, 1, oldFile);

            RecordHeader *nextHdr = (RecordHeader *)nextBuffer;
            if (nextHdr->active == '0') break;

            long tmpNext = nextHdr->next;
            nextHdr->next = -1;
            fwrite(nextBuffer, recordSize, 1, newFile);

            nextOffset = tmpNext;
        }

        free(nextBuffer);
    }

    free(buffer);
    fclose(oldFile);
    fclose(newFile);

    remove(dataFile);
    rename("temp_reorg.bin", dataFile);

    *headOffset = -1;
    *modifications = 0;

    FILE *statusFile = fopen(STATUS_DAT, "wb");
    if (statusFile)
    {
        fwrite(&status, sizeof(Status), 1, statusFile);
        fclose(statusFile);
    }

    createIndex(dataFile, indexFile, recordSize, bptree);
    printf("Reorganização de %s concluída com sucesso.\n", dataFile);
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

        char* enc = encrypt_string(p.categoryAlias);
        strncpy(p.categoryAlias, enc, sizeof(p.categoryAlias) - 1);
        free(enc);

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
