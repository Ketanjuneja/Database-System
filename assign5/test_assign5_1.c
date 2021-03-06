#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"
#include "join_mgr.h"

#define ASSERT_EQUALS_RECORDS(_l,_r, schema, message)			\
  do {									\
    Record *_lR = _l;                                                   \
    Record *_rR = _r;                                                   \
    ASSERT_TRUE(memcmp(_lR->data,_rR->data,getRecordSize(schema)) == 0, message); \
    int i;								\
    for(i = 0; i < schema->numAttr; i++)				\
      {									\
        Value *lVal, *rVal;                                             \
		char *lSer, *rSer; \
        getAttr(_lR, schema, i, &lVal);                                  \
        getAttr(_rR, schema, i, &rVal);                                  \
		lSer = serializeValue(lVal); \
		rSer = serializeValue(rVal); \
        ASSERT_EQUALS_STRING(lSer, rSer, "attr same");	\
		free(lVal); \
		free(rVal); \
		free(lSer); \
		free(rSer); \
      }									\
  } while(0)

#define ASSERT_EQUALS_RECORD_IN(_l,_r, rSize, schema, message)		\
  do {									\
    int i;								\
    boolean found = false;						\
    for(i = 0; i < rSize; i++)						\
      if (memcmp(_l->data,_r[i]->data,getRecordSize(schema)) == 0)	\
	found = true;							\
    ASSERT_TRUE(0, message);						\
  } while(0)

#define OP_TRUE(left, right, op, message)		\
  do {							\
    Value *result = (Value *) malloc(sizeof(Value));	\
    op(left, right, result);				\
    bool b = result->v.boolV;				\
    free(result);					\
    ASSERT_TRUE(b,message);				\
   } while (0)
static void testCreateTableAndInsert (void);
typedef struct TestRecord {
  int a;
  char *b;
  int c;
} TestRecord;

typedef struct TestJoin {
  int a;
  char *b;
  int c;
  char *d;
  int e;
} TestJoin;



char *testName;

Schema *testSchema (void);
Schema *testSchema2 (void);
Schema *testSchema3 (void);
Record *fromTestRecord (Schema *schema, TestRecord in);
Record *fromTestRecord2 (Schema *schema, TestRecord in);
Record *fromTestRecord3 (Schema *schema, TestJoin in);
Record *testRecord(Schema *schema, int a, char *b, int c);
Record *testRecord2(Schema *schema, int a, char *b, int c,char *d,int e);

int 
main (void) 
{
  testName = "";
  testCreateTableAndInsert();
  return 0;
}
void
testCreateTableAndInsert (void)
{
  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
  RM_TableData *table2 = (RM_TableData *) malloc(sizeof(RM_TableData));
  RM_TableData *table3 = (RM_TableData *) malloc(sizeof(RM_TableData));	  
  JoinInfo *j1= (JoinInfo*) malloc(sizeof(JoinInfo));
  JoinInfo *j2= (JoinInfo*) malloc(sizeof(JoinInfo));
  TestRecord inserts[] = { 
    {1, "aaaa", 3}, 
    {2, "bbbb", 2},
    {3, "cccc", 1},
    {4, "dddd", 3},
    {5, "eeee", 5},
    {6, "ffff", 1},
    {7, "gggg", 3},
    {8, "hhhh", 3},
    {9, "iiii", 2}
  };
  TestRecord inserts2[] = { 
    
    {5, "nnnn", 5},
    {6, "oooo", 1},
    {7, "pppp", 3},
    {8, "qqqq", 3},
    {9, "rrrr", 2},
    {10, "qqqq", 8}
  };
  TestJoin inserts3[] = { 
    
    {5, "eeee", 5 , "nnnn", 5},
    {6, "ffff", 1, "oooo", 1},
    {7, "gggg", 3, "pppp", 3},
    {8, "hhhh", 3, "qqqq", 3},
    {9, "iiii", 2, "rrrr", 2}
  };
  int numInserts = 9, i;
  Record *r;
  
  Schema *schema,*schema2,*schema3;
  testName = "test join and union";
  schema = testSchema();
  
  RID *res= (RID *) malloc(sizeof(RID) * numInserts);
  TEST_CHECK(initRecordManager(NULL));
  TEST_CHECK(createTable("test_table_r1.txt",schema));
  TEST_CHECK(openTable(table, "test_table_r1.txt"));
  
  	
  	
  	
  // insert rows into table
  for(i = 0; i < numInserts; i++)
    {
      r = fromTestRecord(schema, inserts[i]);
      TEST_CHECK(insertRecord(table,r)); 
      
    }
    
j1->attrNum=0;
	j1->dt=DT_INT;
  TEST_CHECK(closeTable(table));
  
  // for 2nd relation
   schema2= testSchema2();
   schema3= testSchema3();
  TEST_CHECK(initRecordManager(NULL));
  TEST_CHECK(createTable("test_table_r2.txt",schema2));
  TEST_CHECK(openTable(table2, "test_table_r2.txt"));
  
  TEST_CHECK(initRecordManager(NULL));
  TEST_CHECK(createTable("test_table_r3.txt",schema3));
  TEST_CHECK(openTable(table3, "test_table_r3.txt"));
  
  // insert rows into table
  for(i = 0; i < 6; i++)
    {
      r = fromTestRecord(schema2, inserts2[i]);
      TEST_CHECK(insertRecord(table2,r)); 
      
    }
    
  
	j2->attrNum=0;
	j2->dt=DT_INT;
  TEST_CHECK(closeTable(table2));
  TEST_CHECK(initJoinManager());
  TEST_CHECK(OpenJoinManager(schema,"test_table_r1.txt",schema2,"test_table_r2.txt",schema3,"test_table_r3.txt"));
  
  TEST_CHECK(nested_join(j1,schema,j2,schema2,schema3,res));
   //check result of nested join
  for(i = 0; i < 5; i++)
    {
      
      RID rid = res[i];
      //printf("\n Retrived Data %d %d",rid.page,rid.slot);
      
      TEST_CHECK(getRecord(table3, rid, r));
      
      
      ASSERT_EQUALS_RECORDS(fromTestRecord3(schema3, inserts3[i]), r, schema3, "compare records of Join");
    }
    //union operation
 TEST_CHECK(union_of(schema,schema2));
 
 
  TEST_CHECK(closeJoinManager());
  TEST_CHECK(shutdownJoinManager());
  //free(rids);
  
  TEST_CHECK(closeTable(table3));
  TEST_CHECK(destroyPageFile("test_table_r1.txt"));
  TEST_CHECK(destroyPageFile("test_table_r2.txt"));
  free(table);
  free(table2);
  free(table3);
  
  free(j1);
  free(j2);
  free(res);
  TEST_DONE();
}
Schema *
testSchema (void)	
{
  Schema *result;
  char *names[] = { "a", "b", "c" };
  DataType dt[] = { DT_INT, DT_STRING, DT_INT };
  int sizes[] = { 0, 4, 0 };
  int keys[] = {0};
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * 3);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
  int *cpSizes = (int *) malloc(sizeof(int) * 3);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 3; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], names[i]);
    }
  memcpy(cpDt, dt, sizeof(DataType) * 3);
  memcpy(cpSizes, sizes, sizeof(int) * 3);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}
Schema *
testSchema2 (void)
{
  Schema *result;
  char *names[] = { "a", "d", "e" };
  DataType dt[] = { DT_INT, DT_STRING, DT_INT };
  int sizes[] = { 0, 4, 0 };
  int keys[] = {0};
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * 3);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
  int *cpSizes = (int *) malloc(sizeof(int) * 3);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 3; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], names[i]);
    }
  memcpy(cpDt, dt, sizeof(DataType) * 3);
  memcpy(cpSizes, sizes, sizeof(int) * 3);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}
Schema *
testSchema3 (void)	
{
  Schema *result;
  char *names[] = { "a", "b", "c","d","e" };
  DataType dt[] = { DT_INT, DT_STRING, DT_INT, DT_STRING, DT_INT };
  int sizes[] = { 0, 4, 0,4 ,0 };
  int keys[] = {0};
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * 5);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 5);
  int *cpSizes = (int *) malloc(sizeof(int) * 5);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 5; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], names[i]);
    }
  memcpy(cpDt, dt, sizeof(DataType) * 5);
  memcpy(cpSizes, sizes, sizeof(int) * 5);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(5, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}

Record *
fromTestRecord (Schema *schema, TestRecord in)
{
  return testRecord(schema, in.a, in.b, in.c);
}

Record *
fromTestRecord2 (Schema *schema, TestRecord in)
{
  return testRecord(schema, in.a, in.b, in.c);
}

Record *
fromTestRecord3 (Schema *schema, TestJoin in)
{
  return testRecord2(schema, in.a, in.b, in.c,in.d,in.e);
}


Record *
testRecord(Schema *schema, int a, char *b, int c)
{
  Record *result;
  Value *value;

  TEST_CHECK(createRecord(&result, schema));

  MAKE_VALUE(value, DT_INT, a);
  TEST_CHECK(setAttr(result, schema, 0, value));
  freeVal(value);

  MAKE_STRING_VALUE(value, b);
  TEST_CHECK(setAttr(result, schema, 1, value));
  freeVal(value);

  MAKE_VALUE(value, DT_INT, c);
  TEST_CHECK(setAttr(result, schema, 2, value));
  freeVal(value);

  return result;
}

Record *
testRecord2(Schema *schema, int a, char *b, int c,char *d, int e)
{
  Record *result;
  Value *value;

  TEST_CHECK(createRecord(&result, schema));

  MAKE_VALUE(value, DT_INT, a);
  TEST_CHECK(setAttr(result, schema, 0, value));
  freeVal(value);

  MAKE_STRING_VALUE(value, b);
  TEST_CHECK(setAttr(result, schema, 1, value));
  freeVal(value);

  MAKE_VALUE(value, DT_INT, c);
  TEST_CHECK(setAttr(result, schema, 2, value));
  freeVal(value);
  
  MAKE_STRING_VALUE(value, d);
  TEST_CHECK(setAttr(result, schema, 3, value));
  freeVal(value);

  MAKE_VALUE(value, DT_INT, e);
  TEST_CHECK(setAttr(result, schema, 4, value));
  freeVal(value);

  return result;
}

