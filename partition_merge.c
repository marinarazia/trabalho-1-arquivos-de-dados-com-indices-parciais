/*
int compareOrder(const void *a, const void *b);
int compareProduct(const void *a, const void *b);
void removeDuplicateProducts(const char *sortedFile);

void mergeAllTemps(int tempCount, 
                   size_t recordSize,
                   int (*comparator)(const void *, const void *),
                   const char *finalOutput);

int mergeTwoTemps(const char *file1, 
                 const char *file2, 
                 const char *outFile,
                 size_t recordSize, 
                 int (*comparator)(const void *, const void *));

int createSortedTemps(const char *inputFile, 
                      size_t recordSize,
                      int (*comparator)(const void *, const void *))
*/

int compareOrder(const void *a, const void *b)
{
    const Order *oa = (const Order *)a;
    const Order *ob = (const Order *)b;
    if (oa->id < ob->id) return -1;
    if (oa->id > ob->id) return 1;
    return 0;
}

int compareProduct(const void *a, const void *b)
{
    const Product *pa = (const Product *)a;
    const Product *pb = (const Product *)b;
    if (pa->id < pb->id) return -1;
    if (pa->id > pb->id) return 1;
    return 0;
}

int createSortedTemps(const char *inputFile, 
                      size_t recordSize,
                      int (*comparator)(const void *, const void *))
{
    FILE *in = fopen(inputFile, "rb");
    if (!in) { printf("Erro ao abrir %s\n", inputFile); return 0; }

    void *buffer = malloc(MAX_RECORDS_IN_MEMORY * recordSize);
    if (!buffer) { printf("Erro de memoria.\n"); fclose(in); return 0; }

    int tempCount = 0;
    size_t n;

    while ((n = fread(buffer, recordSize, MAX_RECORDS_IN_MEMORY, in)) > 0)
    {
        qsort(buffer, n, recordSize, comparator);

        char tempName[64];
        sprintf(tempName, "%s_%d.bin", TEMP_PREFIX, tempCount++);
        FILE *out = fopen(tempName, "wb");
        if (!out) { printf("Erro ao criar %s\n", tempName); break; }
        fwrite(buffer, recordSize, n, out);
        fclose(out);
    }

    fclose(in);
    free(buffer);

    printf("%d temps criados para %s.\n", tempCount, inputFile);
    return tempCount;
}

int mergeTwoTemps(const char *file1, 
                  const char *file2, 
                  const char *outFile,
                  size_t recordSize, 
                  int (*comparator)(const void *, const void *))
{
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    FILE *out = fopen(outFile, "wb");

    if (!f1 || !f2 || !out) 
    {
        printf("Erro ao abrir arquivos de merge: %s, %s, %s\n", file1, file2, outFile);
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        if (out) fclose(out);
        return 0;
    }

    void *r1 = malloc(recordSize);
    void *r2 = malloc(recordSize);
    if (!r1 || !r2) { printf("Erro de memoria no merge.\n"); return 0; }

    int has1 = fread(r1, recordSize, 1, f1);
    int has2 = fread(r2, recordSize, 1, f2);

    while (has1 && has2) 
    {
        if (comparator(r1, r2) <= 0)
        {
            fwrite(r1, recordSize, 1, out);
            has1 = fread(r1, recordSize, 1, f1);
        } 
        else 
        {
            fwrite(r2, recordSize, 1, out);
            has2 = fread(r2, recordSize, 1, f2);
        }
    }

    while (has1) { fwrite(r1, recordSize, 1, out); has1 = fread(r1, recordSize, 1, f1); }
    while (has2) { fwrite(r2, recordSize, 1, out); has2 = fread(r2, recordSize, 1, f2); }

    free(r1);
    free(r2);
    fclose(f1);
    fclose(f2);
    fclose(out);
    return 1;
}

void removeDuplicateProducts(const char *sortedFile)
{
    FILE *in = fopen(sortedFile, "rb");
    FILE *out = fopen("pruducts_unique.bin", "wb");

    if (!in || !out) { printf("Erro ao abrir arquivos!\n"); return; }

    Product prev, curr;
    if (fread(&prev, sizeof(Product), 1, in))
    {
        fwrite(&prev, sizeof(Product), 1, out);
        while (fread(&curr, sizeof(Product), 1, in)) 
        {
            if (curr.id != prev.id)
                fwrite(&curr, sizeof(Product), 1, out);

            prev = curr;
        }
    }

    fclose(in);
    fclose(out);

    rename("pruducts_unique.bin", PRODUCT_DAT);
    printf("Arquivo de produtos unicos criado: %s\n", "products_unique.bin");
}

void mergeAllTemps(int tempCount, 
                   size_t recordSize,
                   int (*comparator)(const void *, const void *),
                   const char *finalOutput)
{
    int pass = 0;
    int current = tempCount;
	int i;
	
    while (current > 1) 
    {
        printf("Intercalacao: %d arquivos -> ", current);

        int newCount = 0;
        for (i = 0; i < current; i += 2) {
            char file1[64], file2[64], outFile[64];

            if (pass == 0)
                sprintf(file1, "%s_%d.bin", TEMP_PREFIX, i);
            else
                sprintf(file1, "%s_%d_pass%d.bin", TEMP_PREFIX, i, pass);

            sprintf(outFile, "%s_%d_pass%d.bin", TEMP_PREFIX, newCount, pass + 1);

            if (i + 1 < current)
            {
                if (pass == 0)
                    sprintf(file2, "%s_%d.bin", TEMP_PREFIX, i + 1);
                else
                    sprintf(file2, "%s_%d_pass%d.bin", TEMP_PREFIX, i + 1, pass);

                if (!mergeTwoTemps(file1, file2, outFile, recordSize, comparator))
                    printf("Falha ao mesclar %s e %s\n", file1, file2);

                remove(file1);
                remove(file2);
            } 
            else 
            {
                rename(file1, outFile);
            }
            newCount++;
        }

        current = newCount;
        pass++;
        printf("%d arquivos criados\n", current);
    }

    char finalTemp[64];
    sprintf(finalTemp, "%s_0_pass%d.bin", TEMP_PREFIX, pass);
    rename(finalTemp, finalOutput);

    printf("Arquivo final ordenado criado: %s\n", finalOutput);
}
