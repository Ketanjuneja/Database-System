#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.txt"

/* prototypes for test functions */
static void testWrite(void);
//static void testSinglePageContent(void);

/* main function running all tests */
int
main (void)
{
  testName = "";
  
  initStorageManager();

  testWrite();
//  testSinglePageContent();

  return 0;
}



/* Try to create, open, and close a page file */
void testWrite(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");
  
  
   TEST_CHECK(ensureCapacity(5,&fh)); 
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
    
  
  ASSERT_TRUE((fh.totalNumPages == 5), "expect 5 pages in new file");

  TEST_CHECK(writeBlock (5, &fh, ph));
   
  // read back the page containing the string and check that it is correct
  TEST_CHECK(readBlock (5,&fh, ph));
  
  printf("\nreading :");
  
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");

  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));  
  
  TEST_DONE();
}
