typedef struct {
    ll key;
    long segmentBase; 
} Index;

typedef struct {
    ll id;
    time_t dateTime;
    ll purchasedProductId;
    int skuQty;
    ll userId;
} Order;

typedef struct {
    ll purchasedProductId;
    ll brandId;
    int price; //centavos
    char productGender;
    ll categoryId;
    char categoryAlias[MAX_CATEGORY_ALIAS];
} Product;
