/*

Integrantes: Ricardo Alberti, Marina Razia Goulart Pacheco

Comando para compilar: gcc main.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "entities.h"
#include "helper.c"
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
        printf("4  - Pesquisar produto por id\n");
        printf("5  - Pesquisar ordem por id\n");
        printf("6  - Inserir produto\n");
        printf("7  - Inserir ordem de compra\n");
        printf("8  - Remover ordem de compra\n");
        printf("9  - Remover produto\n");
        printf("10 - Reorganizar arquivo\n");
        printf("0  - Sair\n");
        printf("Escolha: ");
        scanf("%d", &option);
        getchar();

        switch (option) 
        {
            case 1: listOrders(0); break;
            case 2: listProducts(0); break;
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
                insert(PRODUCT_DAT, PRODUCT_INDEX, createNewProduct(), sizeof(Product));
                status.modificationsProduct++;
                break;
			case 7:
                insert(ORDER_DAT, ORDER_INDEX, createNewOrder(), sizeof(Order));
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
				reorganizeFile(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product));
				reorganizeFile(ORDER_DAT, ORDER_INDEX, sizeof(Order));
				break;    
		}
	} while (option != 0);

    FILE *binStatus = fopen(STATUS_DAT, "wb");
    if (binStatus)
    {
        fwrite(&status, sizeof(Status), 1, binStatus);
        fclose(binStatus);
    }

    return 0;
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

        createIndex(ORDER_DAT, ORDER_INDEX, sizeof(Order));
        createIndex(PRODUCT_DAT, PRODUCT_INDEX, sizeof(Product));
    }

    if (binOrder) fclose(binOrder);
    if (binProduct) fclose(binProduct);
    if (indexOrder) fclose(indexOrder);
    if (indexProduct) fclose(indexProduct);
}
