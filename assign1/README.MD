
Problem Statement:

The goal of this assignment is to implement a simple storage manager - a module that is capable of reading blocks 
from a file on disk into memory and writing blocks from memory to a file on disk. The storage manager deals with 
pages (blocks) of fixed size (PAGE_SIZE). In addition to reading and writing pages from a file, it provides methods 
for creating, opening, and closing files. The storage manager has to maintain several types of information for an 
open file: The number of total pages in the file, the current page position (for reading and writing), the file name, 
and a POSIX file descriptor or FILE pointer. 

Steps To Run The Script:

Default test cases:
Compile : cc -o dberror.o ŹŹŹ-c dberror.c
    cc -o storage_mgr.o -c storage_mgr.c  
    cc -o test1.o -c test_assign1_1.c
    cc -o a1 test1.o storage_mgr.o dberror.o
Run: ./a1

Additional test cases:

Compile : cc -o dberror.o -c dberror.c
    cc -o storage_mgr.o -c storage_mgr.c  
    cc -o test2.o -c test2.c
    cc -o a2 test2.o storage_mgr.o dberror.o
Run:./a2


Logic:

Through this code a storage Manager has been implemented. This section provides a gist of what each function does.

createPageFile:
a. Creates a file based on the given fileName
b. 16 bytes of first page is for storing any page related information such as the total number of pages. From 17th byte of first page the actual data starts.

OpenPageFile: 
a. Opens an existing file and retrieves Page information such as Total Number of pages
b. Initializes file handle with the information of the opened file.

closePageFile:
a. Closes the opened file which is associated with the file pointer


destroyPageFile:
a. Destroys the file that is associated with the file pointer

readBlock:
a. Initially checks whether the given page number is valid or not
b. Later checks whether file pointer points to a valid location
c. Reads intended blocks (if page number is zero then start reading from 16th byte)
 
getBlockPos:
a. Gets Current Page Position from currPagePos of filehandler.

readFirstBlock:
a. Reads first block by passing Page Number as 0 to readBlock function.
   
readPreviousBlock:
a. Reads the block previous to the block obtained from Current Page Position i.e. through currPagePos-1

readCurrentBlock:
a. Reads the block by providing the PageNum argument with currPagePos

readLastBlock:
a. Reads the last page by calling function readBlock with parameter pagenum = totalnum_pages

writeBlock: 
a. Verifies the number of pages before writing
b. Start writing from the position given by the user
c. Writes data from memory to the file on disk

writeCurrentBlock: 
a.Writes data from the position obtained via parameter currpagepos.

appendEmptyBlock: 
a. Allocate a block of memory that is equal to the page size
b. Shift the pointer to newly created page
c. Update the total number of pages

ensureCapacity: 
a. Checks if the number of pages in the file are less than what is    required. If so, it calculates the number of pages needed to suffice the requirement
b. Calls appendEmptyblock to append the pages

Additional Test Cases:
Additional test cases are implemented through test2.c. The instructions to run these test cases are provided above.
Features implemented through additional test cases:
1.	Checks working of ensurecapacity
2.	Checks working of appendblock
3.	Checks read/write operation on the appended block


Contributions:

1. Ketan Aswani:
  a. Write related functions and documentation
  b. Additional Test Cases

2. Ketankumar Juneja:
   a. Read related functions and documentation
   b. Additional Test Cases

3. Lasya Ramesh 
  a. Page Related and related documentation
  b. Error Debugging

