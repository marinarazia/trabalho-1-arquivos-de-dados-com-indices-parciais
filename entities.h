// B plus tree
typedef struct BPTNode {
    int isLeaf;
    int numKeys;
    long long keys[BPT_ORDER];
    struct BPTNode *children[BPT_ORDER+1];  // internal nodes
    long offsets[BPT_ORDER];                // leaf nodes
    struct BPTNode *next;                   // leaf chaining
    struct BPTNode *parent;                 // parent pointer
} BPTNode;

typedef struct {
    BPTNode *root;
} BPTree;
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

typedef struct {
    int modificationsOrder;
    int modificationsProduct;
    ll headOrder;
    ll headProduct;
} Status;


extern Status status;
extern BPTree productTree;
extern BPTree orderTree;
