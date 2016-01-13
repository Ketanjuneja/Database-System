#ifndef BTREE_MGR_EX_H
#define BTREE_MGR_EX_H

#include "btree_mgr.h"
#include "storage_mgr.c"
#include "buffer_mgr_ex.c"

typedef struct BTreeExtra {
    BM_BufferManager *bm;
    BTreeFileHeaderPage *btheader;
} BTreeExtra;

typedef struct BTreeFileHeaderPage {
    int root;
    DataType key_type;
    int key_size;
    int value_size;
    int num_nodes;
    int num_entries;
    int num_keys;
} BTreeFileHeaderPage;

typedef struct BTreeNodeHeader {
    BTreeNodeType type;
    int parentid;
    int nkeys;
} BTreeNodeHeader;

typedef enum BTreeNodeType {
    BTN_NODE,
    BTN_LEAF
} BTreeNodeType;

//
extern BTreeNodeHeader *createBTreeNode(BTreeNodeType nodeType, int parentId);
extern BTreeExtra *initBTreeExtra(BM_BufferPool *bm, BTreeFileHeaderPage *btheader);
extern BTreeExtra *getBTreeExtra(BTreeHandle *tree);
extern int keyPos(BTreeHandle *tree, int i);
extern int keyMatch(BTreeHandle *tree, BM_PageHandle *page, BTreeNodeHeader *header, Value *key);
extern int ptrPos(BTreeHandle *tree, int i);
extern int ptrFind(BTreeHandle *tree, BM_PageHandle *page, BTreeNodeHeader *header, Value *key);
extern int ptrGet(BTreeHandle *tree, BM_PageHandle *page, int i);
extern int ridPos(BTreeHandle *tree, int i);
extern RID ridGet(BTreeHandle *tree, BM_PageHandle *page, int i);
extern BTreeNodeHeader *loadBTreeNodeHeader(BM_PageHandle *ph);
extern int findLeafByKey(BTreeHandle *tree, Value *key);

// 
extern RC refreshBTreeHeaderPage(BTreeHandle *tree);


#endif
