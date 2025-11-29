
BPTree bpt_create()
{
    BPTree tree;
    tree.root = NULL;
    return tree;
}

BPTNode* bpt_create_leaf()
{
    BPTNode *node = calloc(1, sizeof(BPTNode));
    node->isLeaf = 1;
    node->numKeys = 0;
    node->next = NULL;
    node->parent = NULL;
    return node;
}

// find leaf where key should go
BPTNode* bpt_find_leaf(BPTree *tree, long long key)
{
    BPTNode *cur = tree->root;
    if (!cur) return NULL;
    while (!cur->isLeaf)
    {
        int i = 0;
        while (i < cur->numKeys && key >= cur->keys[i])
        {
            i++;
        }
        cur = cur->children[i];
    }
    return cur;
}

void bpt_insert_in_leaf(BPTNode *leaf, long long key, long offset)
{
    int i = leaf->numKeys - 1;
    while (i >= 0 && leaf->keys[i] > key)
    {
        leaf->keys[i+1] = leaf->keys[i];
        leaf->offsets[i+1] = leaf->offsets[i];
        i--;
    }

    leaf->keys[i+1] = key;
    leaf->offsets[i+1] = offset;
    leaf->numKeys++;
}

// insert into parent after split
void bpt_insert_in_parent(BPTree *tree, BPTNode *left, long long key, BPTNode *right)
{
    if (left->parent == NULL)
    {
        // create new root
        BPTNode *root = calloc(1, sizeof(BPTNode));
        root->isLeaf = 0;
        root->keys[0] = key;
        root->children[0] = left;
        root->children[1] = right;
        root->numKeys = 1;
        left->parent = root;
        right->parent = root;
        tree->root = root;
        return;
    }

    BPTNode *parent = left->parent;
    // find index to insert new key
    int i;
    for (i = 0; i < parent->numKeys+1; i++)
    {
        if (parent->children[i] == left) break;
    }

    // shift keys and children to make space
    for (int j = parent->numKeys; j > i; j--)
    {
        parent->keys[j] = parent->keys[j-1];
        parent->children[j+1] = parent->children[j];
    }

    parent->keys[i] = key;
    parent->children[i+1] = right;
    right->parent = parent;
    parent->numKeys++;

    // split parent if necessary
    if (parent->numKeys >= BPT_ORDER)
    {
        int mid = BPT_ORDER / 2;
        BPTNode *newInternal = calloc(1, sizeof(BPTNode));
        newInternal->isLeaf = 0;
        newInternal->numKeys = parent->numKeys - mid - 1;

        // copy keys and children to new node
        for (int j = 0; j < newInternal->numKeys; j++)
        {
            newInternal->keys[j] = parent->keys[mid+1 + j];
        }

        for (int j = 0; j <= newInternal->numKeys; j++) 
        {
            newInternal->children[j] = parent->children[mid+1 + j];
            newInternal->children[j]->parent = newInternal;
        }

        long long promotedKey = parent->keys[mid];
        parent->numKeys = mid;

        bpt_insert_in_parent(tree, parent, promotedKey, newInternal);
    }
}

// split a leaf node
void bpt_split_leaf(BPTree *tree, BPTNode *leaf)
{
    int mid = BPT_ORDER / 2;
    BPTNode *newLeaf = bpt_create_leaf();
    newLeaf->numKeys = leaf->numKeys - mid;

    for (int i = 0; i < newLeaf->numKeys; i++)
    {
        newLeaf->keys[i] = leaf->keys[mid + i];
        newLeaf->offsets[i] = leaf->offsets[mid + i];
    }

    leaf->numKeys = mid;
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    newLeaf->parent = leaf->parent;
    long long promotedKey = newLeaf->keys[0];

    bpt_insert_in_parent(tree, leaf, promotedKey, newLeaf);
}

void bpt_insert(BPTree *tree, long long key, long offset)
{
    if (!tree->root)
    {
        tree->root = bpt_create_leaf();
        bpt_insert_in_leaf(tree->root, key, offset);
        return;
    }

    BPTNode *leaf = bpt_find_leaf(tree, key);
    bpt_insert_in_leaf(leaf, key, offset);

    if (leaf->numKeys >= BPT_ORDER)
    {
        bpt_split_leaf(tree, leaf);
    }
}

// search by key
long bpt_search(BPTree *tree, long long key)
{
    BPTNode *leaf = bpt_find_leaf(tree, key);
    if (!leaf) return -1;
    for (int i = 0; i < leaf->numKeys; i++)
    {
        if (leaf->keys[i] == key)
        {
            return leaf->offsets[i];
        }
    }
    return -1;
}
