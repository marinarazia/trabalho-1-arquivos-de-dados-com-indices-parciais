/*

Integrantes: Ricardo Alberti, Marina Razia Goulart Pacheco

Comando para compilar:

gcc main.c -lsodium

Todo: 
 - indice por hash em memoria
 - indice por B-tree em memoria (talvez B+ seja mais fácil)
 - cronometrar tempo de execução das operações e salvar numa tabela excel
 - hash por lista linkado?
*/

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
//
//#include "config.h"
//#include "entities.h"
//#include "crypt.c"
//#include "bpt_index.c"
//#include "helper.c"
//#include "partition_merge.c"
//#include "dataset_processing.c"
//#include "search.c"
//#include "write.c"

#include "crypt.h"
#include "bpt_index.h"
#include "helper.h"
#include "dataset_processing.h"
#include "search.h"
#include "partition_merge.h"
#include "write.h"
#include "search.h"

#include <sodium.h>

Status status = { 0 };
BPTree productTree = { 0 };
BPTree orderTree = { 0 };
HashTable *userHashTable = NULL;

void setupFiles(); 
void benchmarkSearches();

int main() 
{
    if (sodium_init() < 0) 
    {
        printf("Erro ao iniciar biblioteca de criptografia");
        return 1;
    }

    int option;
    int limit;
    ll inputId;

    setupFiles();

    printf("Criando indice hash em memoria...\n");
    userHashTable = createHashIndex(ORDER_DAT, 1000);

    do {
        printf("\n--- MENU ---\n");
        printf("1  - Listar ordens de compra\n");
        printf("2  - Listar produtos\n\n");
        printf("3  - Pesquisar compras de usuario\n\n");
        printf("4  - Pesquisar produto por id\n");
        printf("5  - Pesquisar ordem por id\n\n");
        printf("6  - Inserir produto\n");
        printf("7  - Inserir ordem de compra\n\n");
        printf("8  - Remover ordem de compra\n");
        printf("9  - Remover produto\n\n");
        printf("10 - Reorganizar arquivo\n");
        printf("11 - Benchmark de buscas\n");
        printf("0  - Sair\n");
        printf("Escolha: ");
        scanf("%d", &option);
        getchar();

        switch (option) 
        {
            case 1: 
                printf("Digite um limite: ");
                scanf("%d", &limit);
                listOrders(limit); 
                break;
            case 2: 
                printf("Digite um limite: ");
                scanf("%d", &limit);
                listProducts(limit); 
                break;
            case 3:
                printf("Digite o ID do usuario: ");
                scanf("%lld", &inputId);
                searchOrdersByUser(inputId);
                break;
            case 4:
                printf("Digite o ID do produto: ");
                scanf("%lld", &inputId);
            	searchProductById(inputId);
                break;
            case 5:
            	printf("Digite o ID do pedido: ");
			    scanf("%lld", &inputId);
			    searchOrderById(inputId);
                break;
			case 6:
                insert(PRODUCT_DAT, PRODUCT_INDEX, createNewProduct(), sizeof(Product), &productTree);
                status.modificationsProduct++;
                break;
			case 7:
                insert(ORDER_DAT, ORDER_INDEX, createNewOrder(), sizeof(Order), &orderTree);
                status.modificationsOrder++;
                if (userHashTable) 
                {
                    freeHashTable(userHashTable);
                    userHashTable = createHashIndex(ORDER_DAT, 1000);
                }
                break;
			case 8:
			    printf("Digite o ID do pedido para remover: ");
			    scanf("%lld", &inputId);
			    removeOrder(inputId);
                status.modificationsOrder++;
			    break;
			case 9:
			    printf("Digite o ID do produto para remover: ");
			    scanf("%lld", &inputId);
			    removeProduct(inputId);
                status.modificationsProduct++;
			    break;
			case 10:
				reorganizeFile(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product), &productTree);
				reorganizeFile(ORDER_DAT, ORDER_INDEX, sizeof(Order), &orderTree);
				break;    
            case 11:
                benchmarkSearches();
                break;
		}
	} while (option != 0);

    FILE *binStatus = fopen(STATUS_DAT, "wb");
    if (binStatus)
    {
        fwrite(&status, sizeof(Status), 1, binStatus);
        fclose(binStatus);
    }

    if (userHashTable) 
    {
        freeHashTable(userHashTable);
    }
    return 0;
}

void benchmarkSearches() {
    printf("\nBENCHMARK DE BUSCAS \n");
    
    ll testUsers[] = {1515915625216376356, 1515915625150215144, 1515915625118578609};
    int numTests = 5;
    
    for (int i = 0; i < numTests; i++) {
        printf("\n--- Teste %d: Usuário %lld ---\n", i+1, testUsers[i]);
        
        // Busca com scan (original)
        printf("Busca com SCAN:\n");
        clock_t start = clock();
        int foundScan = searchOrdersByUser(testUsers[i]);
        clock_t end = clock();
        double timeScan = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Tempo SCAN: %.4f segundos\n", timeScan);
        
        // Busca com hash
        printf("Busca com HASH:\n");
        start = clock();
        int foundHash = searchOrdersByUserHash(userHashTable, testUsers[i]);
        end = clock();
        double timeHash = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Tempo HASH: %.4f segundos\n", timeHash);
        
        printf("Resultados: SCAN=%d registros, HASH=%d registros\n", foundScan, foundHash);
        printf("Hash foi %.2fx mais rápido\n", timeScan / timeHash);
    }
}

void setupFiles()
{
    FILE *binOrder = fopen(ORDER_DAT, "rb");
    FILE *binProduct = fopen(PRODUCT_DAT, "rb");
    FILE *indexOrder = fopen(ORDER_INDEX, "rb");
    FILE *indexProduct = fopen(PRODUCT_INDEX, "rb");

    FILE *binStatus = fopen(STATUS_DAT, "rb");
    if (binStatus)
    {
        fread(&status, sizeof(Status), 1, binStatus);
        fclose(binStatus);
    }
    else
    {
        binStatus = fopen(STATUS_DAT, "wb");
        status.headProduct = -1;
        status.headOrder = -1;
        if (binStatus)
        {
            fwrite(&status, sizeof(Status), 1, binStatus);
            fclose(binStatus);
        }
    }

    if (!binOrder || !binProduct || !indexOrder || !indexProduct)
    {
        convertTextToBinary();

        int orderTmps = createSortedTemps(ORDER_DAT, sizeof(Order), compareOrder);
        mergeAllTemps(orderTmps, sizeof(Order), compareOrder, ORDER_DAT);

        int productTmps = createSortedTemps(PRODUCT_DAT, sizeof(Product), compareProduct);
        mergeAllTemps(productTmps, sizeof(Product), compareProduct, PRODUCT_DAT);
        removeDuplicateProducts(PRODUCT_DAT);
    }

    reorganizeFile(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product), &productTree);
    reorganizeFile(ORDER_DAT, ORDER_INDEX, sizeof(Order), &orderTree);

    if (binOrder) fclose(binOrder);
    if (binProduct) fclose(binProduct);
    if (indexOrder) fclose(indexOrder);
    if (indexProduct) fclose(indexProduct);
}
