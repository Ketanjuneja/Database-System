#ifndef JOIN_MGR_H
#define JOIN_MGR_H

#include "dberror.h"
#include "expr.h"
#include "tables.h"
#include "buffer_mgr.h"
#include "record_mgr.h"

typedef struct JoinInfo {
  
  int attrNum;
  DataType dt;
 	
} JoinInfo;

extern RC initJoinManager ();
extern RC shutdownJoinManager ();
extern RC OpenJoinManager (Schema *s1,char *t1,Schema *s2,char *t2,Schema *s3,char *t3);
extern RC closeJoinManager ();
extern RC nested_join(JoinInfo *j1,Schema *s1,JoinInfo *j2,Schema *s2,Schema *s3,RID *res);
extern RC union_of(Schema *s1,Schema *s2);

#endif

