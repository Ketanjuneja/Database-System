#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_mgr.h"
#include "dberror.h"
#include<math.h>
#include "storage_mgr.h"

#define MAX_PAGES 20000
#define MAX_FRAMES 200
#define MAX_K 10

/* Data structure for every page in the burffer pool */

typedef struct pageFrameNode{
    
    int pageNum;            /* page Number of the page in the pagefile */
    int frameNum;           /* frame number of the frame in the buffer pool */
    int dirty;              
    int fixCount;           /* fixCount of the page based on the pinning/un-pinning request*/
    int rf;                 /* reference bit per node to be used by clock*/
    int pageFrequency;     /* frequency of the page per client request for LFU*/
    char *data;             /* data contained in the frame */
    struct pageFrameNode *next;
    struct pageFrameNode *previous;
    
}pageFrameNode;

/*  frame list with a pointer to head and tail node of type pageFrameNode.*/
typedef struct frameList{
    
    pageFrameNode *head;    
    pageFrameNode *tail;    
    
}frameList;

/* Data structure which handles all information of a buffer pool(This is connected to bm->mgmtData)*/
typedef struct bmInfo{
   
    int numOfWrite;           
    int numOfRead;            
    int numOfFrames;          /* number of frames in the frame list that are occupied */
    int pinCount;             /* number of pinning done for the pool */
    void *stratData;
    int pageToFrame[MAX_PAGES];         /* an array from pageNumber to frameNumber. The size should be same as the size of the pageFile.*/
    int frameToPage[MAX_FRAMES];        /* an array from frameNumber to pageNumber. The size should be same as the size of the frame list.*/
    int pageToFrequency[MAX_PAGES];     /* an array mappping pageNumber to pageFrequency. The size should be same as the size of the pageFile.*/
    bool dirtyFlags[MAX_FRAMES];        
    int fixedCounts[MAX_FRAMES];        
    int khist[MAX_PAGES][MAX_K];        /* history of reference of each page in memory*/
    frameList *frames;      		
    
}bmInfo;


/****************** Initializtion of a new node in the buffer pool ***********************/
 pageFrameNode *newNode(){
    
    
    
    pageFrameNode *node = malloc(sizeof(pageFrameNode));
    node->dirty = 0;
    node->fixCount = 0;
    node->rf = 0;
    node->pageNum = NO_PAGE;
    node->frameNum = 0;
    node->data =  calloc(PAGE_SIZE, sizeof(SM_PageHandle));
    node->previous = NULL;
    node->next = NULL;
    node->pageFrequency = 0;
    
    return node;
}

/****************** Initializtion of the buffer pool ***********************
  1. Initializes all pool variables.
  2. Sets default value in the mapping arrays
  3. Creates frames from the given numPages
 ***************************************************************************/
  
 RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData)
{
    int i;
    SM_FileHandle fh;
    if(numPages <= 0)
    {
        return RC_INVALID;
    }
    
    if (openPageFile ((char *)pageFileName, &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
    }
    
    /* Initialize the data for mgmtInfo.*/
    bmInfo *pool = malloc(sizeof(bmInfo));
    
    pool->numOfFrames = 0;
    pool->numOfRead = 0;
    pool->numOfWrite = 0;
    pool->stratData = stratData;
    pool->pinCount = 0;
    
    /* Initialize the lookup arrays with 0 values.*/
    memset(pool->frameToPage,NO_PAGE,MAX_FRAMES*sizeof(int));
    memset(pool->pageToFrame,NO_PAGE,MAX_PAGES*sizeof(int));
    memset(pool->dirtyFlags,NO_PAGE,MAX_FRAMES*sizeof(bool));
    memset(pool->fixedCounts,NO_PAGE,MAX_FRAMES*sizeof(int));
    memset(pool->khist, -1, sizeof(&(pool->khist)));
    memset(pool->pageToFrequency,0,MAX_PAGES*sizeof(int));
    
    /* Create the linked list of empty frames. */
    pool->frames = malloc(sizeof(frameList));
    
    pool->frames->head = pool->frames->tail = newNode();
    
    for(i = 1; i<numPages; ++i){
        pool->frames->tail->next = newNode();
        pool->frames->tail->next->previous = pool->frames->tail;
        pool->frames->tail = pool->frames->tail->next;
        pool->frames->tail->frameNum = i;
    }

    bm->pageFile = (char*) pageFileName;
    bm->strategy = strategy;    
    bm->numPages = numPages;
    bm->mgmtData = pool;
    
    closePageFile(&fh);
    return RC_OK;
}

/*************** Flushing of memory **************************
 1. Checks validity of pages
 2. Traverses through the pool and checks if any page is dirty
 3. If yes then write it to the memory and traverse the pool further
**************************************************************/
 
 RC forceFlushPool(BM_BufferPool *const bm)
{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID;
    }
    
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *current = pool->frames->head;
    
    SM_FileHandle fh;
    
    if (openPageFile ((char *)(bm->pageFile), &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
    }
    
    while(current != NULL){
        if(current->dirty == 1){
        	//printf("\n page num=%d ",current->pageNum);
            if(writeBlock(current->pageNum, &fh, current->data) != RC_OK){
                return RC_WRITE_FAILED;
            }
            current->dirty = 0;
            (pool->numOfWrite)++;
        }
        current = current->next;
    }
    
    closePageFile(&fh);
    
    return RC_OK;
}

/******************* Shut Down BufferPool ********************
1. Checks validity of the pages
2. Calls force flush pool and checks whether it returns RC_OK status
3. Closes the buffer pool and clears the memory 
***************************************************************/

RC shutdownBufferPool(BM_BufferPool *const bm)
{
     if (!bm || bm->numPages <= 0){
        return RC_INVALID;
    }
    RC status;
    
    if((status = forceFlushPool(bm)) != RC_OK){
        return status;
    }
    
     bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *current = pool->frames->head;
    
    while(current != NULL){
        current = current->next;
        free(pool->frames->head->data);
        free(pool->frames->head);
        pool->frames->head = current;
    }
    
    pool->frames->head = pool->frames->tail = NULL;
    free(pool->frames);
    free(pool);
    
    bm->numPages = 0;
    
    return RC_OK;
}
/****************** Search Page by PAge Number ******************
1. Checks if the given page number exists in the frame list 
2. If found returns the node/frame else returns NULL
*****************************************************************/
 pageFrameNode *searchPageByNum(frameList *frames, const PageNumber pageNum)
{
	pageFrameNode *current= frames->head;
	
	while(current != NULL)
	{
		if(current->pageNum == pageNum)
		{
			return current;
		}
		
		current = current->next;
	}
	return NULL;
}

/************************ MArk Dirty **************************
1. Checks for validity of the pages
2. Searches for the page in the buffer pool
3. If found, marks it dirty and returns RC_OK status.
***************************************************************/

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID;
    }
    
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *found;
    
    found = searchPageByNum(pool->frames, page->pageNum);
    if(found == NULL){
        return RC_NON_EXISTING_PAGE_IN_FRAME;
    }
    
    /* Mark the page as dirty */
     found->dirty = 1;
    
    return RC_OK;
}

/******************** Unpin a page *****************************
1. Checks for validity of the pages
2. Searches for the page in the frame list of the pool
3. If found it unpins the page and deccreases the fixCount
****************************************************************/
 
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID;
    }
    
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *found;
    
    /* Locate the page to be unpinned */
    if((found = searchPageByNum(pool->frames, page->pageNum)) == NULL){
        return RC_NON_EXISTING_PAGE_IN_FRAME;
    }
    
    /* Unpinned so decrease the fixCount by 1 */
    if(found->fixCount > 0){
        found->fixCount--;
    }
    else{
        return RC_NON_EXISTING_PAGE_IN_FRAME;
    }
    
    return RC_OK;
}

/************************ Force Page ************************
1. Checks for validity of the pages 
2. Locates the page to be written on the disk
3. If found, does write operation of the page on the disk
*************************************************************/

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)

{
    if (bm== NULL){
        return RC_INVALID;
    }
    
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *found;
    SM_FileHandle fh;
    
    if (openPageFile ((char *)(bm->pageFile), &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
    }
    
    /* Locate the page to be forced on the disk */
     if((found = searchPageByNum(pool->frames, page->pageNum)) == NULL){
        closePageFile(&fh);
        return RC_NON_EXISTING_PAGE_IN_FRAME;
        
    }
    
    /* write the current content of the page back to the page file on disk */
    if(writeBlock(found->pageNum, &fh, found->data) != RC_OK){
        closePageFile(&fh);
        return RC_WRITE_FAILED;
    }
    
    (pool->numOfWrite)++;
    
    closePageFile(&fh);
    
    return  RC_OK;
}

/******************** Insert Page ************************
Inserts a frame in the frame list i.e. in the buffer pool
**********************************************************/

 void insertNode(frameList **list, pageFrameNode *newNode){
    
    pageFrameNode *head = (*list)->head;
    
    if(newNode == (*list)->head || head == NULL ){
	   if(newNode == NULL)
        return;
        return;
    }
    else if(newNode == (*list)->tail){
        pageFrameNode *temp = ((*list)->tail)->previous;
        temp->next = NULL;
        (*list)->tail = temp;
        newNode->next = head;
    	head->previous = newNode;
    	newNode->previous = NULL;
    	(*list)->head = newNode;
    	(*list)->head->previous = NULL;
    }
    else{
        newNode->previous->next = newNode->next;
        newNode->next->previous = newNode->previous;
       newNode->next = head;
    	head->previous = newNode;
    	newNode->previous = NULL;
    	(*list)->head = newNode;
    	(*list)->head->previous = NULL;
    }
    
    
    
    return;
    
}
/*************************** Get Page ***************************
1. Searches for the page in the frame list of the pool
2. Returns the details to the user
3. Increases fix count and sets frequency 
*****************************************************************/

 pageFrameNode *pageInMemory(BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum){
	    
    pageFrameNode *found;
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    
    if((pool->pageToFrame)[pageNum] != NO_PAGE){
        if((found = searchPageByNum(pool->frames, pageNum)) == NULL){
            return NULL;
        }
       	page->pageNum = pageNum;
        page->data = found->data; 
        found->fixCount++;
        found->rf = 1;
        return found;
    }
    return NULL; 
}


 RC updateNewFrame(BM_BufferPool *const bm, pageFrameNode *found, BM_PageHandle *const page, const PageNumber pageNum)
 {
    RC status;
    SM_FileHandle fh;
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    
    status = openPageFile ((char *)(bm->pageFile), &fh);
     if (status != RC_OK){
        return status;
    }
    
    
     if(found->dirty == 1){
     status = ensureCapacity(pageNum, &fh);
        if(status != RC_OK){
            return status;
        }
        status = writeBlock(found->pageNum,&fh, found->data);
        if(status != RC_OK){
            return status;
        }
        (pool->numOfWrite)++;
    }
    
    
    (pool->pageToFrame)[found->pageNum] = NO_PAGE;
    
    
    status = ensureCapacity(pageNum, &fh);
     if(status != RC_OK){
        return status;
    }
    status = readBlock(pageNum, &fh, found->data);
    if(status != RC_OK){
        return status;
    }
    
    
    page->pageNum = pageNum;
    page->data = found->data; 
    (pool->numOfRead)++;
    found->dirty = 0;
    found->fixCount = 1;
    found->pageNum = pageNum;
    found->rf = 1;
    
    (pool->pageToFrame)[found->pageNum] = found->frameNum;
    (pool->frameToPage)[found->frameNum] = found->pageNum;
    
    closePageFile(&fh);
    
    return RC_OK;
    
}

/********************** FIFO Strategy ****************************
1. Checks if the page is availabe in the buffer pool. 
2. If found, it makes it the head of the frame list in the pool
3. If not, Calls for insert page function and inserts the page on the head 
********************************************************************/
 
RC pinPage_FIFO (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    
    pageFrameNode *found;
    bmInfo *pool = (bmInfo *)bm->mgmtData;
   
    /* Check if page already in memory. The pageToFrame array provides the fast lookup.*/
    if((found = pageInMemory(bm, page, pageNum)) != NULL){
        return RC_OK;
    }
    
    /* If need to load the page*/
    
    
    /* If the frames in the memory are less than the total available frames, find out the first free frame from the head. */
    if((pool->numOfFrames) < bm->numPages){
        found = pool->frames->head;
        int i = 0;
        
        while(i < pool->numOfFrames){
            found = found->next;
            ++i;
        }
        
        /*This frame will be filled up now, so increase the frame count*/
        (pool->numOfFrames)++;
        
        insertNode(&(pool->frames), found);
        
    }
    else{
        /* For new page, if all the frames are filled, find the oldest frame with fix count 0 */
     found = pool->frames->tail;
        
        while(found != NULL && found->fixCount != 0){
            found = found->previous;
        }
        
        if (found == NULL){
            return RC_BUFFER_FULL;
        }
        
        insertNode(&(pool->frames), found);
    }
    
    RC status;
    
    status = updateNewFrame(bm, found, page, pageNum);
    
    if( status!= RC_OK){
    
        return status;
    }
    
    return RC_OK;
}

/******************************** LRU Strategy ************************************
1. Checks if the page is availabe in the buffer pool. 
2. If found, it makes it the head of the frame list in the pool
3. If not, Calls for insert Node and deletes the least recently used from the tail of the pool
************************************************************************************/ 

RC pinPage_LRU (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    pageFrameNode *found;
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    
    /* Check if page already in memory. The pageToFrame array provides the fast lookup.*/
    if((found = pageInMemory(bm, page, pageNum)) != NULL){
        /* Put this frame to the head of the frame list, because it is the latest used frame. */
        insertNode(&(pool->frames), found);
        return RC_OK;
    }
    
    /* If need to load the page*/
    
    /* If the frames in the memory are less than the total available frames, find out the first free frame from the head. */
    if((pool->numOfFrames) < bm->numPages){
        found = pool->frames->head;
        
        int i = 0;
        while(i < pool->numOfFrames){
            found = found->next;
            ++i;
        }
        /*This frame will be filled up now, so increase the frame count*/
        (pool->numOfFrames)++;
    }
    else{
        /* For new page, if all the frames are filled, then try to find a frame with fixed count 0
         * starting from the tail.*/
        found = pool->frames->tail;
        
        while(found != NULL && found->fixCount != 0){
            found = found->previous;
        }
        
        /* If reached to head, it means no frames with fixed count 0 available.*/
        if (found == NULL){
            return RC_BUFFER_FULL;
        }
    }
    
    RC status;
    
    if((status = updateNewFrame(bm, found, page, pageNum)) != RC_OK){
        return status;
    }
    
    /* Put this frame to the head of the frame list, because it is the latest used frame. */
    insertNode(&(pool->frames), found);
    
    return RC_OK;
}

/****************************** Pin Page *******************************
1. Checks for validity of the Page number 
2. Checks replacement strategy and calls the appropriate function
************************************************************************/
    
 RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
            const PageNumber pageNum)
{
    if (!bm || bm->numPages < 0){
        return RC_INVALID;
    }

    if(bm->strategy == RS_FIFO)
            return pinPage_FIFO(bm,page,pageNum);
            
    else if(bm->strategy == RS_LRU)
            return pinPage_LRU(bm,page,pageNum);
          
    else
            return RC_UNKNOWN_STRATEGY;
    
    return RC_OK;
}   

PageNumber *getFrameContents (BM_BufferPool *const bm)
{
    /*return the value of frameToPage*/
    return ((bmInfo *)bm->mgmtData)->frameToPage;
 }

 bool *getDirtyFlags (BM_BufferPool *const bm)
{
    /*traverses the list of frames and updates the value of dirty flag*/
    bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *current = pool->frames->head;
    
    while (current != NULL){
        (pool->dirtyFlags)[current->frameNum] = current->dirty;
        current = current->next;
    }
    
    return pool->dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm)
{
    /*goes through the list of frames and updates the value of fixedCounts accordingly*/
     bmInfo *pool = (bmInfo *)bm->mgmtData;
    pageFrameNode *current = pool->frames->head;
    
    while (current != NULL){
        (pool->fixedCounts)[current->frameNum] = current->fixCount;
        current = current->next;
    }
    
    return pool->fixedCounts;
}

int getNumReadIO (BM_BufferPool *const bm)
{
    /*returns the value of numOfRead*/
    return ((bmInfo *)bm->mgmtData)->numOfRead;
}

int getNumWriteIO (BM_BufferPool *const bm)
{
    /*returns the value of numOfWrite*/
    return ((bmInfo *)bm->mgmtData)->numOfWrite;
}
