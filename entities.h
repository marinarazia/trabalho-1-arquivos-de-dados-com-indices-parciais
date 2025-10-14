typedef struct {
    ll key;
    long segmentBase; 
} Index;

typedef struct {
    ll id;
    ll userId;
    ll purchasedProductId;
    time_t dateTime;
    int skuQty;
    char active; // '1' = ativo, '0' = excluido
} Order;

typedef struct {
    ll purchasedProductId;
    ll categoryId;
    ll brandId;
    int price; // em centavos
    char productGender;
    char categoryAlias[MAX_CATEGORY_ALIAS];
    char active; // '1' = ativo, '0' = excluido
} Product;
