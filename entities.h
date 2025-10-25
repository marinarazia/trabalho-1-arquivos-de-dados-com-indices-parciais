typedef struct {
    int modificationsProduct;
    int modificationsOrder;
    ll currentExtensionId;
} Status;

typedef struct {
    ll id; // id of last record in the segment
    long position; // segment start position
} Index;

typedef struct {
    ll id;
    ll userId;
    ll purchasedProductId;
    time_t dateTime;
    int skuQty;
    char active; // '1' = ativo, '0' = excluido
    ll next; // -1 = fim
} Order;

typedef struct {
    ll id;
    ll categoryId;
    ll brandId;
    int price; // em centavos
    char productGender;
    char categoryAlias[MAX_CATEGORY_ALIAS];
    char active; // '1' = ativo, '0' = excluido
    ll next; // -1 = fim
} Product;

extern Status status;
