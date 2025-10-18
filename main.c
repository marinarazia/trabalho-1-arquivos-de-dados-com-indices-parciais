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
    [x] completar switch no main 

    [] pesquisa binária
    [] add elo para inserção por area de extensão
    [] (Ricardo) função reestruturar indices e arquivo
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

static int modificationCount = 0;

int main() 
{
    int option = 0;
    ll inputId = 0;

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
        printf("3 - Pesquisar compras de usuário\n");
        printf("4 - Pesquisar produto por id\n");
        printf("5 - Inserir ordem de compra\n");
        printf("6 - Inserir produto\n");
        printf("7 - Remover ordem de compra\n");
        printf("8 - Remover produto\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &option);
        getchar();

        if (modificationCount > MAX_MODIFICATIONS)
        {
            restructureDataset();
            modificationCount = 0;
        }

        switch (option) 
        {
            case 1: listOrders(5); break;
            case 2: listProducts(5); break;
            case 3:
                printf("Digite o ID do usuário: ");
                scanf("%lld", &inputId);
                searchOrdersByUser(inputId);
                break;
            case 4:
                printf("Digite o ID do produto: ");
                scanf("%lld", &inputId);
                searchProductById(inputId);
                break;            
            case 5:
			    Order newOrder = createNewOrder();
			    insertOrder(newOrder);
                modificationCount++;
			    break;
			case 6:
			    Product newProduct = createNewProduct();
			    insertProduct(newProduct);
                modificationCount++;
			    break;
			case 7:
			    printf("Digite o ID do pedido para remover: ");
			    scanf("%lld", &inputId);
			    removeOrder(inputId);
                modificationCount++;
			    break;
			case 8:
			    printf("Digite o ID do produto para remover: ");
			    scanf("%lld", &inputId);
			    removeProduct(inputId);
                modificationCount++;
			    break;
		}
	} while (option != 0);

    return 0;
}
