#define DEBUG 0

// Dataset source
#define DATASET_CSV "jewelry.csv"

// Data Files
#define STATUS_DAT "status.bin"
#define ORDER_DAT "orders.bin"
#define PRODUCT_DAT "products.bin"

// Partial index files
#define ORDER_INDEX "ordersIdx.bin"
#define PRODUCT_INDEX "productsIdx.bin"

// B+ index
#define MIN_DEGREE 6
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MIN_KEYS (MIN_DEGREE - 1)
#define MAX_CHILDREN (2 * MIN_DEGREE)
#define MIN_CHILDREN MIN_DEGREE

// Hash index
//
//
//
//

// Cryptography 
#define SECRET_KEY "ibQ2_iXNaW95MBXCuAgd3qluAa0nK1qg" // 32B
#define DECRYPT_ON_READ 1

#define TEMP_PREFIX "temp"
#define MAX_RECORDS_IN_MEMORY 1024
#define SEGMENT_SIZE 10
#define MAX_MODIFICATIONS 1000
#define MAX_CATEGORY_ALIAS 128
#define ll long long
