Assignment Number 2

Problem Statement:

The goal of this assignment is to implement a buffer manager. The buffer manager manages a fixed number of pages in memory that represent pages from a page file managed by the storage manager implemented in assignment 1. The memory pages managed by the buffer manager are called page frames or frames for short. We call the combination of a page file and the page frames storing pages from that file a Buffer Pool. The Buffer manager should be able to handle more than one open buffer pool at the same time. However, there can only be one buffer pool for each page file. Each buffer pool uses one page replacement strategy that is determined when the buffer pool is initialized.At least two replacement strategies FIFO and LRU to be implemented.

How to Run the Script:
Compile: make assign2_1
Run: ./assign2_1
To revert On terminal: make clean
Logic:

Data Structures and Design:
We have implemented the frames as a doubly-linked list. Each node of the frames is called pageFrameNode, and it contains below attributes:
pageNum      - the page number of the page in the pageFile.
frameNum     - the number of the frame in the frame list.
dirty        - the dirty bit of the frame.( 1 = dirty, 0 = not dirty).
fixCount     - fixCount of the page based on the pinning/un-pinning request.
data         - actual data pointed by the frameNode.

Few attributes are defined or required at BufferPool level. A structure of these attributes called poolInfo is implementer by assigning it to the BM_BufferPool->mgmtData. The attributes of this structure are:
1. numOfFrames    - filled number of frames in the frame list.
2. numOfRead      - total number of read done on the buffer pool.
3. numOfWrite     - total number of write done on the buffer pool.
4. pinCount - total number of pinning done for the buffer pool.
5. stratData    - value of BM_BufferPool->stratData
6. pageToFrame  - an array from pageNumber to frameNumber. Value : FrameNum, if page in memory; otherwise -1.
7. frameToPage  - an array from frameNumber to pageNumber. Value : PageNum, if frame is full; otherwise -1.
8. dirtyFlags  - an array of dirtyflags of all the frames.
9. fixedCounts - an array of fixed count of all the frames.

Buffer Pool Functions :

initBufferPool:
1. BM_BufferPool instance is taken as an argument by this function and attributes are initialized based on other arguments.
2. Arguments provided are validated.  If any of them are invalid (for example, numPages <= 0), appropriate error messages are returned.
3. Frame List is initialized with empty frames

shutdownBufferPool:
1. BM_BufferPool instance is taken as an argument and memory is deallocated.
2. Error messages are returned for invalid arguments.
3. Frames from the frame list are emptied and memory from each node is de allocated.
4. Memory with BM_BufferPool->mgmtData which refers to the bmInfo structure is de allocated at the end.

forceFlushPool:
1. All dirty frames are written to the file on disk through this function. This is accomplished by taking BM_BufferPool instance as an argument.
2. Returns an error in case of invalid arguments.
3. It then checks frame list to find a frame that has a dirty bit 1. If any found then does the following:
   a. Data is written back to the file on disk.
   b. dirty bit is set to 0.
   c. Value of numWrite is increased by 1;
4. When the check is complete for all frames, it returns RC_OK if no error faced.

Page Management Functions:
pinPage:
1. This function implements pinpage function defined as per different strategies like: pinpage_FIFO, pinpage_LRU, etc.
2. These functions pins the page with the given pagenumber pageNum.
3. Different strategies are used by buffer manager to locate the page requested and details of the page to client. 

unpinPage:
1. This function first iterate through the available pages in the frames to locate the page to be unpinned.
2. If the page is not found it returns an exception "RC_NON_EXISTING_PAGE_IN_FRAME"
3. If the page is found, "fixCount" is decreased by 1 and the function returns RC_OK.

markDirty:
1. This function first iterate through the available pages in the frames to locate the page to be marked as dirty.
2. If the page is found then set the dirty bit of the page node as 1 and return RC_OK.

forcePage:
1. This function first iterate through the available pages in the frames to locate the page to be forced to disk.
2. If the page is found it opens the file and write current content of the page back to the page file on disk.
3. If page is found and the write operation is successful it returns RC_OK otherwise returns RC_NON_EXISTING_PAGE_IN_FRAME.

Statistics Functions:
Statistics functions provide information regarding the buffer pool and its contents.
The print debug functions provide information about the pool using statistics function.
getFrameContents:
1. An array of PageNumbers(frameToPage) where the ith element is the page stored in the ith page frame, is included in BM_BufferPool->mgmtData. This is updated whenever a new frame is added in the function updateNewFrame.
   getFrameContents returns this array.

getDirtyFlags:
1. An array of booleans are returned by the function, where the ith element is true if ith page is dirty.
2. This array is stored in BM_bufferPool->mgmtData->dirtyFlags. getDirtyFlags returns this array.
3. List of frames are traversed and dirty flags are populated and frames are checked to see which frames are dirty.

getFixCounts:
1. This function returns an array of ints where the ith element is the fix count of the page stored in the ith page frame.
2. 0 is returned for empty page frames.
3. This array is stored in BM_bufferPool->mgmtData->fixedCounts. getFixCounts returns this array.
4. By traversing the list of frames and using the fixCount value of each frame variable fixedCounts is populated

getNumReadIO:
1. Number of pages that have been read from disk since a buffer pool was initialized is returned by this function.
2. The value of BM_bufferPool->mgmtData->numRead which is set in updateNewFrame and in initBufferPool is returned by the function.

getNumWriteIO:
1. The number of pages that have been written to the page file since the buffer pool was initialized is returned.
2. The value of BM_bufferPool->mgmtData->numWrite which is set in updateNewFrame, initBufferPool,
   forcePage, and forceFlushPool is returned here.

The Page Replacement Strategies:
pinPage_FIFO:
FIFO page replacement strategy is implemented here.
1. A check is done to see if the page is in memory. It calls pageInMemory function If the page is found described in "Helper Functions" later, and returns RC_OK.
2. if the page is required to be loaded in the memory,The first free frame is located starting from head 
   When an empty frame is found the page is loaded in the found frame and page details are set. 
  The found frame is updated to be the head of the linked list.
3. Oldest frame is located with fix count zero. The found frame is updated to be the head of the list.
4. If the frame is not found following above strategy then the function returns no more space in buffer error. Else performs updateNewFrame function 

pinPage_LRU:
LRU replacement policy is implemented. 
1. First verifies whether the page is in memory. If present calls the pageInMemory function described in
   Helper Functions later, and returns immediately with RC_OK.
2. So, at any moment the head will be the latest used frame, and the tail will be the least used frame because every time a frame is referenced, it is moved to the head of the framelist.
3. If the page is not in memory, it searches for a frame with fixcount 0 from the tail of framelist.
4. updateNewFrame function is called when any such frame is found described in Helper Functions later; otherwise no more space in buffer error is returned


Helper Functions:
4 helper functions mainly used are as follows:
insertNode:
1. This function takes a framelist and a framenode as arguments, and makes the node the head of given list.
2. Different page replacement strategies use this function in order to keep the logical order of the frames in the frame list.
findNodeByPageNum:
1. framelist and a pageNumber are arguments to this function, and searches for the page in the framelist.
2. frameNode is returned If the page is found; otherwise returns NULL.
3. Different page replacement strategies use this function in order to find the required frame from the frame list.

pageInMemory:
1. BM_BufferPool, BM_PageHandle and a pageNumber are arguments to this function. it searches for the page in the framelist.
2. If the page is found :
   a. set the BM_PageHandle to refer to this page in memory.
   b. increase the fixcount of the page.
3. used by different page replacement strategies when the page is already available in the frame list.

updateNewFrame:
1. BM_BufferPool, BM_PageHandle, the destination frame and a pageNumber are arguments to this function.
2. when the destination frame has a dirty page, writes the page back to the disk and updates related attributes.
3. The destination page from the disk into memory is read, and stored in the given frame.
4. pageNum, numRead, dirty, fixcount and rf attributes are updated of the destination frame.
5. Different page replacement strategies use this function in case when the page is not in the memory. It calls the function with the frame to be replaced, and the new page to be loaded.

No memory leaks:
The essential and the additional test cases are implemented and tested with valgrind for no memory leaks.

Contribution    
1. Ketankumar Juneja :
  a. Statistic Functions.
  b. Page Replacement Strategies: LRU
  c. Checking valgrind/memory leak issues
  d. Data structure and design.

2. Ketan Aswani:
  a. Page Management Related Functions 
  b. Page Replacement Strategies : FIFO
  c. Related test cases and documentation
  d. Checking valgrind/memory leak issues


3. Lasya Ramesh:
  a. Buffer Pool Functions.
  b. Related test cases and documentation
  c. Additional Error checks.
  d. Related test cases and documentation.

