#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "record_mgr.h"




typedef struct recordmgmt {
    Expr *cond;
    int slot;
    int page;
    
}recordmgmt;




extern int getRecordSize (Schema *schema){
    int size = 0, tempSize = 0, i;

    for(i=0; i<schema->numAttr; ++i){
        switch (schema->dataTypes[i]){
            case DT_STRING:
                tempSize = schema->typeLength[i];
                break;
            case DT_INT:
                tempSize = sizeof(int);
                break;
            case DT_FLOAT:
                tempSize = sizeof(float);
                break;
            case DT_BOOL:
                tempSize = sizeof(bool);
                break;
            default:
                break;
        }
        size += tempSize;
    }
    return size;
}
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){

    Schema *schema = (Schema *) malloc(sizeof(Schema));

    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keySize = keySize;
    schema->keyAttrs = keys;

    return schema;
}
extern RC freeSchema (Schema *schema){
    free(schema);
    return RC_OK;
}
extern RC initRecordManager (void *mgmtData)
{
    return RC_OK;
}
extern RC shutdownRecordManager (){
    return RC_OK;
}

char * serializemgmtinfo(tablemgmt *mgmt)
{

char * str = (char *) calloc (PAGE_SIZE,sizeof(char));

sprintf(str,"FirstPage [%d] LastPage [%d] Tuplescount [%d] SlotSize [%d] maxSlots [%d] ;\n",mgmt->FirstPage, mgmt->LastPage, mgmt->Tuplescount, mgmt->slotsize, mgmt->maxSlots);

return str;

}
extern RC createTable (char *name, Schema *schema){

  

    int return_value,slot_size=0;
    SM_FileHandle fh;
        
    //int schema_size = getSchemaSize(schema);
    tablemgmt *mgmt = (tablemgmt *) malloc(sizeof(tablemgmt));
     slot_size += (1 + 5 + 1 + 5 + 1 + 1 + 1); // 2 square brackets, 2 int, 1 hyphen, 1 space, 1 bracket.
    int i=0,temp;
    while(i<schema->numAttr)
    {
        if( schema->dataTypes[i]==DT_INT || schema->dataTypes[i]==DT_BOOL)
        {
               temp=5+ strlen(schema->attrNames[i]);
         }
         else if(schema->dataTypes[i]==DT_STRING)
         {
                temp= schema->typeLength[i] + strlen(schema->attrNames[i]);
                
          }
            
          else
           {
                temp= 10+ strlen(schema->attrNames[i]);
            }
            
        slot_size += (temp + 1 + 1);     //  colon, coma or ending bracket.
        i++;
    }
	mgmt->slotsize=slot_size;
   // mgmt->slotsize = slotSize(schema);
    
    mgmt->maxSlots = (int) floor((float)PAGE_SIZE/(float)slot_size);
	mgmt->Tuplescount = 0;
	mgmt->FirstPage = 1;
	mgmt->LastPage = 1;
	mgmt->tstone_root = NULL;
	
	//char* tbinfo,*scinfo;
	
	char * tbinfo= serializemgmtinfo(mgmt);
	 char* scinfo= serializeSchema(schema);
	
	strcat(tbinfo,scinfo);
	strcat(tbinfo,"  ; \n " );
		free(scinfo);	
	return_value=createPageFile(name);
	//printf(" createPageFile %d",return_value);
    if ( return_value != RC_OK)
        return return_value;

	return_value=openPageFile(name, &fh);
    if (return_value != RC_OK)
        return return_value;

    if ((return_value=ensureCapacity((2), &fh)) != RC_OK){
        return return_value;
    }
    
    return_value=writeBlock(0, &fh,tbinfo);
    if (return_value != RC_OK)
        return return_value;
	free(tbinfo);
    /* From next page, write the schema*/
    return_value=closePageFile(&fh);
    if ( return_value!= RC_OK)
        return return_value;
	
    return RC_OK;
}
extern int getNumTuples (RM_TableData *rel){
    return ((tablemgmt *)rel->mgmtData)->Tuplescount;
}
void calcoffset(Schema *schema, int attrNum,int *x)
{
   int addr=0,i=0;
   while(i<attrNum)
   {
   
     if(DT_INT==schema->dataTypes[i])
     {
        addr+=sizeof(int);
       }

	else if(DT_STRING==schema->dataTypes[i])
     {
        addr+=schema->typeLength[i];
       }

	else if(DT_BOOL==schema->dataTypes[i])
     {
        addr+=sizeof(bool);
       }	
       
	else
     {
        addr+=sizeof(float);
       }
       i++;
     }
     *x=addr;
  
  }	
  
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{

	int return_value;
	
	calcoffset(schema,attrNum,&return_value);
	 (*value)=(Value*)malloc(sizeof(Value));
	  char *data,*str;
	  
	  
	data=record->data + return_value;
	
	
	if(DT_INT==schema->dataTypes[attrNum])
     {
       	(*value)->dt = DT_INT;
	memcpy(&((*value)->v.intV),data,sizeof(int));
      }

	else if(DT_STRING==schema->dataTypes[attrNum])
	{
     	//	printf("\n Getting string");
        (*value)->dt = DT_STRING;
      	str=malloc(schema->typeLength[attrNum]+1);
      	int strl=schema->typeLength[attrNum];
      	strncpy(str, data, strl ); 
      	str[strl]='\0';
     	//printf("\n Here value is %s",str);
      	(*value)->v.stringV = str;
        
       }

	else if(DT_BOOL==schema->dataTypes[attrNum])
     {
        (*value)->dt = DT_BOOL;
      	str=malloc(sizeof(bool)+1);
      	strncpy(str, data, sizeof(bool)); 
      	str[sizeof(bool)]='\0';
      
      	//(*value)->v.boolV=(bool*) str;
       }	
       
	else
     {
        memcpy( (&(*value)->v.floatV),data,sizeof(float));
        (*value)->dt=DT_FLOAT;
       }
	return RC_OK;
}

extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	int return_value;
	 calcoffset(schema,attrNum,&return_value);
	char *data,*str;  
	data=record->data + return_value;
	
	if(DT_INT==schema->dataTypes[attrNum])
     	{
       		value->dt = DT_INT;
      		memcpy(data,&(value->v.intV),sizeof(int));
      	}

	else if(DT_STRING==schema->dataTypes[attrNum])
     	{
        
      		str=(char *)malloc(schema->typeLength[attrNum]);
      		str=value->v.stringV; 
      		
     	  	memcpy(data,str,schema->typeLength[attrNum]);
     
      		      
       	}

	else if(DT_BOOL==schema->dataTypes[attrNum])
     	{
        	value->dt = DT_BOOL;
      		memcpy(data,&(value->v.boolV),sizeof(bool));	
      		
       	}	
       
	else
     	{
        	
        	memcpy(data,&(value->v.floatV),sizeof(float));
        	value->dt=DT_FLOAT;
       	}
 	
	return RC_OK;
}
extern RC createRecord (Record **record, Schema *schema){

	/* allocate memory for a new record and record data as per the schema. */
    *record = (Record *)  malloc(sizeof(Record));
    (*record)->data = (char *)malloc((getRecordSize(schema)));

    return RC_OK;
}
extern RC freeRecord (Record *record){
	 /* free the memory space allocated to record and its data */
    free(record->data);
    free(record);

    return RC_OK;
}
RC updateTableInfo(tablemgmt *info,char *name)
{
	char* tbinfo,*temp1;
	int return_value;
	char * str = (char *) calloc (PAGE_SIZE,sizeof(char));
	char *str1= (char*) calloc(100,sizeof(char));
	 tbinfo= serializemgmtinfo(info);
	SM_FileHandle fh;
	tNode *troot=info->tstone_root;
	
	return_value=openPageFile(name, &fh);
    if (return_value != RC_OK)
        return return_value;

    
    return_value=readBlock(0, &fh,str);
    if (return_value != RC_OK)
        return return_value;
    
    
   memcpy(str,tbinfo,strlen(tbinfo));
    
    return_value=writeBlock(0, &fh,str);    /* Struct attributes are written, now we will write tstone info*/
    if (return_value != RC_OK)
        return return_value;

    	
    	if(troot!=NULL)
    	
    	{
    	
    	
	return_value=readBlock(0, &fh,str);
    	if (return_value != RC_OK)
        return return_value;
        
        temp1=strtok(str,";");
        temp1=strtok(NULL,";");
		        
	temp1+=10;
	    
	    tNode * temp=troot;
	    
	    while(temp!=NULL)
	    {
			sprintf(str1,"[%d:%d]",temp->page,temp->slot);
    		}
    		strcat(str1,";");
    		memcpy(temp1,str1,strlen(str1));
    
    	return_value=writeBlock(0, &fh,str);    
    	if (return_value != RC_OK)
        return return_value;
    
    
    	}
    
    
    return_value=closePageFile(&fh);
    if ( return_value!= RC_OK)
        return return_value;
	free(str);
	free(str1);
    return RC_OK;
    


}
tablemgmt* deserializeMgmtInfo(char *page){
    tablemgmt *mgmtinfo =(tablemgmt*)malloc(sizeof(tablemgmt));
    char *temp1, *temp2; 	
    char mgmtdata[strlen(page)];
    strcpy(mgmtdata, page);


    //printf("\n Schema attr mgmt %s",mgmtdata);
    temp1 = strtok (mgmtdata,"[");
    temp1 = strtok (NULL,"]");
    mgmtinfo->FirstPage = strtol(temp1, &temp2, 10);

    temp1 = strtok (NULL,"[");
    temp1 = strtok (NULL,"]");
    mgmtinfo->LastPage = strtol(temp1, &temp2, 10);

    
    temp1 = strtok (NULL,"[");
    //printf("\n Schema attr %s ",temp1);
    temp1 = strtok (NULL,"]");
    mgmtinfo->Tuplescount = strtol(temp1, &temp2, 10);
    //   printf("\n Schema attr SchemaSize");	
    
    temp1 = strtok (NULL,"[");
    temp1 = strtok (NULL,"]");
    mgmtinfo->slotsize = strtol(temp1, &temp2, 10);

    temp1 = strtok (NULL,"[");
    temp1 = strtok (NULL,"]");
    mgmtinfo->maxSlots = strtol(temp1, &temp2, 10);

    
    
    
   
   /* temp1 = strtok (NULL,"<");
    temp1 = strtok (NULL,">");
    int tnode_len = strtol(temp1, &temp2, 10);*/
  //  mgmtinfo->tNodeLen = tnode_len;
    temp1 = strtok (mgmtdata,";");
    temp1 = strtok (NULL,";");
    temp1 = strtok (NULL,"[");
    int  pages, slot;

    mgmtinfo->tstone_root = NULL;
    tNode *temp_node;

    while(temp1!=NULL){
        temp1 = strtok (NULL,":");
        strcat(temp1,"\0");
        pages = atoi(temp1);

        /*if(i != (tnode_len - 1)){
            temp1 = strtok (NULL,",");
        }*/
       
            temp1 = strtok (NULL,"]");
       		strcat(temp1,"\0");
        slot = atoi(temp1);
	
        if (mgmtinfo->tstone_root == NULL){
            mgmtinfo->tstone_root = (tNode *)malloc(sizeof(tNode));
            mgmtinfo->tstone_root->page = pages;
            mgmtinfo->tstone_root->slot = slot;
            temp_node = mgmtinfo->tstone_root;
        }
        else{
            temp_node->next = (tNode *)malloc(sizeof(tNode));
            temp_node->next->page = pages;
            temp_node->next->slot = slot;

            temp_node = temp_node->next;
        }
	temp1 = strtok (NULL,"[");
    }
    return mgmtinfo;
    
}
Schema* deserializeSchema(char *schema_str){
  Schema *schema = (Schema *) malloc(sizeof(Schema));
    int i, j;

    char schema_data[strlen(schema_str)];
    strcpy(schema_data, schema_str);

    char *temp1, *temp2;
    temp1 = strtok (schema_data,"<");
    temp1 = strtok (NULL,">");

    int numAttr = strtol(temp1, &temp2, 10);
    //printf("\n Deserialized numAttr %d",numAttr);
    schema->numAttr= numAttr;
	//printf("\n Deserialized numAttr %d",schema->numAttr);
    schema->attrNames=(char **)malloc(sizeof(char*)*numAttr);
    schema->dataTypes=(DataType *)malloc(sizeof(DataType)*numAttr);
    schema->typeLength=(int *)malloc(sizeof(int)*numAttr);
    char* str_ref[numAttr];
    temp1 = strtok (NULL,"(");

    for(i=0; i<numAttr; ++i){
        temp1 = strtok (NULL,": ");
        schema->attrNames[i]=(char *)calloc(strlen(temp1), sizeof(char));
        strcpy(schema->attrNames[i], temp1);

        if(i == numAttr-1){
            temp1 = strtok (NULL,") ");
        }
        else{
            temp1 = strtok (NULL,", ");
        }

        str_ref[i] = (char *)calloc(strlen(temp1), sizeof(char));

        if (strcmp(temp1, "INT") == 0){
            schema->dataTypes[i] = DT_INT;
            schema->typeLength[i] = 0;
        }
        else if (strcmp(temp1, "FLOAT") == 0){
            schema->dataTypes[i] = DT_FLOAT;
            schema->typeLength[i] = 0;
        }
        else if (strcmp(temp1, "BOOL") == 0){
            schema->dataTypes[i] = DT_BOOL;
            schema->typeLength[i] = 0;
        }
        else{
            strcpy(str_ref[i], temp1);
        }
       // printf("Datatype is : %d", schema->dataTypes[i]);
    }

    int keyFlag = 0, keySize = 0;
    char* keyAttrs[numAttr];

    if((temp1 = strtok (NULL,"(")) != NULL){
        temp1 = strtok (NULL,")");
        char *key = strtok (temp1,", ");

        while(key != NULL){
            keyAttrs[keySize] = (char *)malloc(strlen(key)*sizeof(char));
            strcpy(keyAttrs[keySize], key);
            keySize++;
            key = strtok (NULL,", ");
        }
        keyFlag = 1;
    }

    char *temp3;
    for(i=0; i<numAttr; ++i){
        if(strlen(str_ref[i]) > 0){
            temp3 = (char *) malloc(sizeof(char)*strlen(str_ref[i]));
            memcpy(temp3, str_ref[i], strlen(str_ref[i]));
            schema->dataTypes[i] = DT_STRING;
            temp1 = strtok (temp3,"[");
            temp1 = strtok (NULL,"]");
            schema->typeLength[i] = strtol(temp1, &temp2, 10);
            free(temp3);
            free(str_ref[i]);
        }
    }

    if(keyFlag == 1){
        schema->keyAttrs=(int *)malloc(sizeof(int)*keySize);
        schema->keySize = keySize;
        for(i=0; i<keySize; ++i){
            for(j=0; j<numAttr; ++j){
                if(strcmp(keyAttrs[i], schema->attrNames[j]) == 0){
                    schema->keyAttrs[i] = j;
                    free(keyAttrs[i]);
                }
            }
        }
    }

    return schema;
}
extern RC openTable (RM_TableData *rel, char *name){


    BM_BufferPool *bm = (BM_BufferPool *)malloc(sizeof(BM_BufferPool));
    BM_PageHandle *page =  (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
	char *temp1=(char*) calloc(PAGE_SIZE,sizeof(char));
	char *temp2;
	int x;
	//Schema *s= (Schema*)malloc(sizeof(Schema)); 
    if((x=initBufferPool(bm, name, 3, RS_FIFO, NULL))!=RC_OK)
    {
    return x;
    }
    pinPage(bm, page, 0);
    tablemgmt *mgmtinfo = deserializeMgmtInfo(page->data);
    strcpy(temp1,page->data);
    temp2=strtok(temp1,";");
    temp2=strtok(NULL,";");
	
     rel->schema=deserializeSchema(temp2);
    
    //printf("\n Rel datatype %d",rel->schema->dataTypes[0]);
    rel->name = name;
	//printf("\n Dt types in open %d %d %d",rel->schema->dataTypes[0],rel->schema->dataTypes[1],rel->schema->dataTypes[2]);
    mgmtinfo->bp = bm;
    rel->mgmtData =(void *) mgmtinfo;

    free(page);
	free(temp1);
    return RC_OK;
}

extern RC closeTable (RM_TableData *rel){

    shutdownBufferPool(((tablemgmt *)rel->mgmtData)->bp);
    
    free(rel->mgmtData);

    free(rel->schema->dataTypes);

    free(rel->schema->attrNames);

    free(rel->schema->keyAttrs);
    free(rel->schema->typeLength);

    free(rel->schema);

    return RC_OK;
}


extern RC insertRecord (RM_TableData *rel, Record *record)
{
    int slotid, pageid,return_value;
   
    tablemgmt *info = (tablemgmt *) (rel->mgmtData);
    BM_BufferPool *bm = (BM_BufferPool *)info->bp;
    BM_PageHandle *page = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));

    slotid= info->Tuplescount % info->maxSlots;
    pageid=(info->Tuplescount/info->maxSlots) + 1; 	
    //printf("\n Slotid %d pageid %d Data : %s",slotid,pageid,record->data);
    record->id.page = pageid;
    record->id.slot = slotid;
    //printf("\n ok");
    
    
    char *record_str = serializeRecord(record, rel->schema);
   // printf("\n Serialized record %s ",record_str);
    //printf("\n Dt types %d %d %d",rel->schema->dataTypes[0],rel->schema->dataTypes[1],rel->schema->dataTypes[2]);
    pinPage(bm, page, pageid);
    memcpy(page->data + (slotid*info->slotsize), record_str, strlen(record_str));
    
    free(record_str);
    
    
    markDirty(bm, page);
    unpinPage(bm, page);
    forcePage(bm, page);
    
    (info->Tuplescount)++;
    if(pageid> info->LastPage)
    info->LastPage=pageid;
    
   return_value=updateTableInfo(info,rel->name);
	   // printf("\n Dt types after update %d %d %d",rel->schema->dataTypes[0],rel->schema->dataTypes[1],rel->schema->dataTypes[2]);
   
    if (return_value != RC_OK)
        return return_value;
    free(page); 
    return RC_OK; 
     

}
Record * deserializeRecord(char *record_str, RM_TableData *rel){

    Schema *schema = rel->schema;
    Record *record = (Record *) malloc(sizeof(Record));
    tablemgmt *info = (tablemgmt *) (rel->mgmtData);
    Value *value;
    record->data = (char *)malloc(sizeof(char) * info->slotsize);
    char record_data[strlen(record_str)];
    strcpy(record_data, record_str);
    char *temp1, *temp2;
	//printf("Datatype %d NumAttr %d",schema->dataTypes[0],schema->numAttr);
    temp1 = strtok(record_data,"-");

    /* int page; */

    /* page = strtol(temp1+1, &temp2, 10); */

    temp1 = strtok (NULL,"]");
    /* int slot; */
    /* slot = strtol(temp1, &temp2, 10); */
    temp1 = strtok (NULL,"(");

    int i;

    for(i=0; i<schema->numAttr; ++i){
        temp1 = strtok (NULL,":");
        if(i == (schema->numAttr - 1)){
            temp1 = strtok (NULL,")");
        }
        else{
            temp1 = strtok (NULL,",");
        }
		//printf("\n Dt types %d %d %d",schema->dataTypes[0],schema->dataTypes[1],schema->dataTypes[2]);
        /* set attribute values as per the attributes datatype */
        switch(schema->dataTypes[i]){
            case DT_INT:
            {

                int val = strtol(temp1, &temp2, 10);
		//	printf("\n Int value %d",val);
                MAKE_VALUE(value, DT_INT, val);
                setAttr (record, schema, i, value);
		freeVal(value);
            }
                break;
            case DT_STRING:
            {
               // printf("\n In string");
                MAKE_STRING_VALUE(value, temp1);
                setAttr (record, schema, i, value);
                
               // printf("\n Value %s",value->v.stringV);
		freeVal(value);
            }
                break;
            case DT_FLOAT:
            {
                float val = strtof(temp1, NULL);
                MAKE_VALUE(value, DT_FLOAT, val);
                setAttr (record, schema, i, value);
		freeVal(value);
            }
                break;
            case DT_BOOL:
            {
                bool val;
                val = (temp1[0] == 't') ? TRUE : FALSE;
                MAKE_VALUE(value, DT_BOOL, val);
                setAttr (record, schema, i, value);
		freeVal(value);
            }
                break;
        }
    }
    free(record_str);
    return record;
}
extern RC getRecord (RM_TableData *rel, RID id, Record *record){
    tablemgmt *info = (tablemgmt *) (rel->mgmtData);
    BM_PageHandle *page = (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
	//Schema *s= rel->schema;
    int page_num, slot;
    page_num = id.page;
    slot = id.slot;

    record->id.page = page_num;
    record->id.slot = slot;
    tNode *temp=info->tstone_root;
	//printf("\n Dt types in get %d %d %d",s->dataTypes[0],s->dataTypes[1],s->dataTypes[2]);
	while(temp!=NULL)
	{
	   if(temp->page==page_num && temp->slot==slot)
	   return RC_DELETED_TUPLE;
	   
	   temp=temp->next;
	 }
 
        pinPage(info->bp, page, page_num);
        
        char *record_str = (char *) malloc(sizeof(char) * info->slotsize);
        memcpy(record_str, page->data + ((slot)*info->slotsize), sizeof(char)*info->slotsize);
        //printf("\n retrived record %s",record_str);
        unpinPage(info->bp, page);
	//printf("\n ok");
	//printf("\n Dt types %d %d %d",rel->schema->dataTypes[0],rel->schema->dataTypes[1],rel->schema->dataTypes[2]);
        Record *temp_record=deserializeRecord(record_str, rel);

        record->data = temp_record->data;
        //free(temp_record);
    

    free(page);
    return RC_OK;
}
extern RC deleteTable (char *name){
    if(access(name, F_OK) == -1) {
        return RC_TABLE_NOT_FOUND;
    }

    if(remove(name) != 0){
        return RC_TABLE_NOT_FOUND;
    }
    return RC_OK;
}
 extern RC updateRecord (RM_TableData *rel, Record *record)
{
    BM_PageHandle *page = (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
    tablemgmt *info = (tablemgmt *) (rel->mgmtData);
    int page_num, slot;

    page_num = record->id.page;
    slot = record->id.slot;
	tNode *temp=info->tstone_root;
	
	while(temp!=NULL)
	{
	   if(temp->page==page_num && temp->slot==slot)
	   return RC_DELETED_TUPLE;
	   
	   temp=temp->next;
	 }
	   
    char *record_str = serializeRecord(record, rel->schema);

    pinPage(info->bp, page, page_num);
    memcpy(page->data + (slot*info->slotsize), record_str, strlen(record_str));
    free(record_str);

    markDirty(info->bp, page);
    unpinPage(info->bp, page);
    forcePage(info->bp, page);
	free(page);
 //   tableInfoToFile(rel->name, info);

    return RC_OK;

}
extern RC deleteRecord (RM_TableData *rel, RID id){
    
    
    tNode *temp = (tNode *)malloc(sizeof(tNode));
    temp->next=NULL;
    tablemgmt *data=(tablemgmt *) (rel->mgmtData);
    tNode *temp2=data->tstone_root;
        /* add deleted RID to end of tombstone list */
        if(temp2!= NULL){
        
        	while (temp2->next != NULL){
                temp2 = temp2->next;
            }
            temp->page = id.page;
           temp->slot = id.slot;
            free(temp);
        }
        else
        {
            
            temp2 = (tNode *)malloc(sizeof(tNode));
            //tstone_iter = tstone_iter->next;
            temp2->next = NULL;
           temp2->page = id.page;
           temp2->slot = id.slot;
        	free(temp2);
        	free(temp);
        }

        
        (data->Tuplescount)--;
        updateTableInfo(data,rel->name);
    
    

    return RC_OK;

}
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){

	
    scan->rel = rel;

	
    recordmgmt *sinfo = (recordmgmt *) malloc(sizeof(recordmgmt));
    sinfo->page = 1;
    sinfo->slot = 0;
    sinfo->cond = cond;
   
	
    scan->mgmtData =  sinfo;

    return RC_OK;
}
extern RC next (RM_ScanHandle *scan, Record *record){

    
    Value *value;
    recordmgmt *sinfo = scan->mgmtData;
    
   // record=(Record*)malloc(sizeof(Record));
    tablemgmt *tinfo=(tablemgmt *) scan->rel->mgmtData;
	int maxslots=tinfo->maxSlots;
   //int firstpage=tinfo->FirstPage;
    //Value *v;
    int lastpage=tinfo->LastPage;
    int count =0;

    record->id.slot = sinfo->slot;
    record->id.page = sinfo->page;

	
    tNode *temp=tinfo->tstone_root;
	
	while(temp!=NULL)
	{
	   if(temp->page==sinfo->page && temp->slot==sinfo->slot)
	   {
	          if (sinfo->slot!=(maxslots-1)  ){
            			sinfo->slot++;
            			
        		}
        	else
        	{
        	
        	    sinfo->slot=0;
        	    sinfo->page++;
        	    if(sinfo->page > lastpage)
        	    {
        	    	//printf("\n Getting here %d=%d", page,lastpage);
        	      return RC_DELETED_TUPLE;
        	    }
        	  }
	   	scan->mgmtData=sinfo;
	   	return next(scan,record);
	   
	   }
	   temp=temp->next;
	   count++;
	}
		int x= (sinfo->page - tinfo->FirstPage)*(tinfo->maxSlots) + sinfo->slot + 1 - count;
		if(x > tinfo->Tuplescount)
		return RC_RM_NO_MORE_TUPLES;	
    if(RC_OK != getRecord(scan->rel, record->id, record)){

        return RC_RM_NO_MORE_TUPLES;
    }

	
    
	
        evalExpr(record, scan->rel->schema, sinfo->cond, &value);
        if (sinfo->slot == maxslots - 1){
            sinfo->slot = 0;
            (sinfo->page)++;
        }
        else{
            (sinfo->slot)++;
        }
        scan->mgmtData = sinfo;

	
        if(value->v.boolV != 1){
        //printf("\n Rescanned ");
        return next(scan, record);
        }
        else{
        //	printf("\n Everythings fine");
            return RC_OK;
        
            
        }
    

}
extern RC closeScan (RM_ScanHandle *scan){
    //free(scan);
    return RC_OK;
}
