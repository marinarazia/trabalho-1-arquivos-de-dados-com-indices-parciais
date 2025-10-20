/*
Integrantes: Ricardo Alberti, Marina Razia

Comando para compilar: gcc main.c

TODO:
    [x] arquivo de dados para binario
    [x] arquivo de dados ordenado
    [x] funcao criar arquivo indice parcial
    [x] arquivo de dados produtos nao duplicado

    [x] listagem dos dados
    [x] funcoes de consulta
    [x] funcoes de remocao
    [x] funcoes de insercao
        - (sugestao: remocao logica e insercao por area de extensao)

    [x] completar switch no main

    pra fazer a area de extensao talvez precise adicionar mais um campo
    nos structs em entities.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "entities.h"
#include "dataset_processing.c"
#include "partition_merge.c"
#include "search.c"
#include "insert.c"

int main()
{
    int option;
    ll inputId;

    convertTextToBinary();

    int tempOrders = createSortedTemps(BIN_ORDER, sizeof(Order), compareOrder, "temp_orders");
    int tempProducts = createSortedTemps(BIN_PRODUCT, sizeof(Product), compareProduct, "temp_products");
    mergeAllTemps(tempOrders, sizeof(Order), compareOrder, "temp_orders", BIN_ORDER);
    mergeAllTemps(tempProducts, sizeof(Product), compareProduct, "temp_products", BIN_PRODUCT);

    removeDuplicateProducts(BIN_PRODUCT, "products_unique.bin");

    FILE *indexOrder = fopen(INDEX_ORDER, "rb");
    FILE *indexProduct = fopen(INDEX_PRODUCT, "rb");

    if (!indexOrder || !indexProduct)
    {
        createIndex();
    }
    else
    {
        fclose(indexOrder);
        fclose(indexProduct);
    }

    do {
        printf("\n--- MENU ---\n");
        printf("1 - Listar ordens de compra\n");
        printf("2 - Listar produtos\n");
        printf("3 - Pesquisar compras de usuario\n");
        printf("4 - Pesquisar produto por id (busca binaria)\n");
        printf("5 - Pesquisar ordem com elo de extensao (busca binaria)\n");
        printf("6 - Inserir ordem de compra\n");
        printf("7 - Inserir produto\n");
        printf("8 - Inserir ordem com elo de extensao\n");
        printf("9 - Remover ordem de compra\n");
        printf("10 - Remover produto\n");
        printf("11 - Reorganizar arquivo\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &option);
        getchar();

        switch (option)
        {
            case 1: listOrders(5); break;
            case 2: listProducts(5); break;
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
            case 6:
            	{
            		Order newOrder = createNewOrder();
				    insertOrder(newOrder);
				    break;
				}
			    
			case 7:
				{
					Product newProduct = createNewProduct();
				    insertProduct(newProduct);
				    break;
				}
			    
			case 8:
				{
					Order newOrder = createNewOrder();
					insertOrderWithExtension(newOrder);
					break;
				}
			
			case 9:
			    printf("Digite o ID do pedido para remover: ");
			    scanf("%lld", &inputId);
			    removeOrder(inputId);
			    break;
			case 10:
			    printf("Digite o ID do produto para remover: ");
			    scanf("%lld", &inputId);
			    removeProduct(inputId);
			    break;
			    
			case 11:
				reorganizeOrderFile();
				break;    
		}
	} while (option != 0);

    return 0;
}
