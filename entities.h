// B plus tree
typedef struct BPTNode {
    int isLeaf;  // 4B
    int numKeys; // 4B
    long long keys[BPT_ORDER]; // 8B * ORDER
    struct BPTNode *children[BPT_ORDER+1]; // 8B * (ORDER+1)
    long offsets[BPT_ORDER];  // 8B * ORDER
    struct BPTNode *next;     // 8B             
    struct BPTNode *parent;   // 8B
} BPTNode;  // ORDER 169 = 4096B cada nó

typedef struct {
    BPTNode *root;
} BPTree;

typedef struct HashEntry {
    ll key;
    long file_offset;
    struct HashEntry *next;
} HashEntry;

typedef struct {
    int size;
    int count;
    HashEntry **table;
} HashTable;

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
extern HashTable *hashTable;
