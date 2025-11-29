typedef struct {
    int modificationsOrder;
    int modificationsProduct;
    ll headOrder;
    ll headProduct;
} Status;

// -----------
// B+
typedef struct {

} Node;
// -----------

typedef struct {
    ll id; // id of last record in the segment
    long position; // segment start position
} Index;

typedef struct {
    ll id;
    char active; // '1' = ativo, '0' = excluido
    ll next; // -1 = fim

    ll userId;
    ll purchasedProductId;
    time_t dateTime;
    int skuQty;
} Order;

typedef struct {
    ll id;
    char active; // '1' = ativo, '0' = excluido
    ll next; // -1 = fim

    ll categoryId;
    ll brandId;
    int price; // em centavos
    char productGender;
    char categoryAlias[MAX_CATEGORY_ALIAS];
} Product;

typedef struct {
    ll id;
    char active;
    ll next;
} RecordHeader;

extern Status status;
