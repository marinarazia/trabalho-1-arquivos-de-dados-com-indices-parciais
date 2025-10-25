/*
Integrantes: Ricardo Alberti, Marina Razia Goulart Pacheco

Comando para compilar: gcc main.c

TODO: 
    [x] arquivo de dados para binario
    [x] arquivo de dados ordenado
    [x] funcao criar arquivo indice parcial
    [x] arquivo de dados produtos distintos

    [x] listagem dos dados
    [x] funcoes de consulta
    [x] funcoes de remocao
    [x] funcoes de insercao
    [x] completar switch no main 

    [x] pesquisa binária
    [x] add elo para inserção por area de extensão
    [x] (Ricardo) função reestruturar indices e arquivo
    []corrigir inserção e pesquisa
    []data e hora
    []revisar reorganize file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "entities.h"
#include "partition_merge.c"
#include "dataset_processing.c"
#include "search.c"
#include "write.c"

Status status = { 0 };

void setupFiles(); 

int main() 
    {
    int option;
    ll inputId;

    setupFiles();

    do {
        printf("\n--- MENU ---\n");
        printf("1  - Listar ordens de compra\n");
        printf("2  - Listar produtos\n");
        printf("3  - Pesquisar compras de usuario\n");
        printf("4  - Pesquisar produto por id (busca binaria)\n");
        printf("5  - Pesquisar ordem com elo de extensao (busca binaria)\n");
        printf("6  - Inserir produto\n");
        printf("7  - Inserir ordem de compra\n");
        printf("8  - Remover ordem de compra\n");
        printf("9  - Remover produto\n");
        printf("10 - Reorganizar arquivo\n");
        printf("0  - Sair\n");
        printf("Escolha: ");
        scanf("%d", &option);
        getchar();

        if (status.modificationsOrder > MAX_MODIFICATIONS)
        {
            reorganizeOrderFile();
        }
        else if (status.modificationsProduct > MAX_MODIFICATIONS)
        {
            reorganizeProductFile();
        }

        switch (option) 
        {
            case 1: listOrders(10); break;
            case 2: listProducts(10); break;
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
			    searchOrderByIdWithExtension(inputId);
                break;
			case 6:
                insertProduct(createNewProduct());
                status.modificationsProduct++;
                break;
			case 7:
                insertOrder(createNewOrder());
                status.modificationsOrder++;
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
				reorganizeOrderFile();
				reorganizeProductFile();
				break;    
		}
	} while (option != 0);

    FILE *binStatus = fopen(BIN_STATUS, "wb");
    if (binStatus)
    {
        fwrite(&status, sizeof(Status), 1, binStatus);
        fclose(binStatus);
    }

    return 0;
}

void setupFiles()
{
    FILE *binOrder = fopen(BIN_ORDER, "rb");
    FILE *binProduct = fopen(BIN_PRODUCT, "rb");
    FILE *indexOrder = fopen(INDEX_ORDER, "rb");
    FILE *indexProduct = fopen(INDEX_PRODUCT, "rb");

    FILE *binStatus = fopen(BIN_STATUS, "rb");
    if (binStatus)
    {
        fread(&status, sizeof(Status), 1, binStatus);
        fclose(binStatus);
    }
    else
    {
        binStatus = fopen(BIN_STATUS, "wb");
        if (binStatus)
        {
            status.currentExtensionId = EXTENSION_AREA_START;
            fwrite(&status, sizeof(Status), 1, binStatus);
            fclose(binStatus);
        }
    }

    if (!binOrder || !binProduct || !indexOrder || !indexProduct)
    {
        convertTextToBinary();

        int orderTmps = createSortedTemps(BIN_ORDER, sizeof(Order), compareOrder);
        mergeAllTemps(orderTmps, sizeof(Order), compareOrder, BIN_ORDER);

        int productTmps = createSortedTemps(BIN_PRODUCT, sizeof(Product), compareProduct);
        mergeAllTemps(productTmps, sizeof(Product), compareProduct, BIN_PRODUCT);
        removeDuplicateProducts(BIN_PRODUCT);

        createIndex(BIN_ORDER, INDEX_ORDER, sizeof(Order));
        createIndex(BIN_PRODUCT, INDEX_PRODUCT, sizeof(Product));
    }

    if (binOrder) fclose(binOrder);
    if (binProduct) fclose(binProduct);
    if (indexOrder) fclose(indexOrder);
    if (indexProduct) fclose(indexProduct);
}
