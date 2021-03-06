#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include "tables.h"
#include "join_mgr.h"

// Three mgmtInfo variables related to the three relations to be dealt with 
tablemgmt *mgmtinfo,*mgmtinfo2,*mgmtinfo3;
RM_TableData *table,*table2,*table3;

void printu(Schema *s1,Record *r1,Value *v1);
void print(Schema *s1,Record *r1,Value *v1,Record *r2,Schema *s2);
void print2(Schema *s1,Record *r1,Value *v1,Record *r2,Schema *s2,int x,int a2);

// Initialization of Join Manager
extern RC initJoinManager ()
{
    return RC_OK;
}


extern RC shutdownJoinManager (){
	
  	   return RC_OK;
}

/******** Open the Join Manager to perform operations on the given tables ***********
** Initializes the variables and open all the tables
*************************************************************************************/

extern RC OpenJoinManager (Schema *s1,char *t1,Schema *s2,char *t2,Schema *s3,char *t3){

	mgmtinfo =(tablemgmt*)malloc(sizeof(tablemgmt));
	mgmtinfo2 =(tablemgmt*)malloc(sizeof(tablemgmt));
	table = (RM_TableData *) malloc(sizeof(RM_TableData));
 	table2 = (RM_TableData *) malloc(sizeof(RM_TableData));
 	table3 = (RM_TableData *) malloc(sizeof(RM_TableData));
 	openTable(table, t1);
  	mgmtinfo=table->mgmtData;
  	openTable(table2, t2);
  	mgmtinfo2=table2->mgmtData;
  	openTable(table3, t3);
  	mgmtinfo3=table3->mgmtData;
    return RC_OK;
}

/************ Closes all tables ****************/

extern RC closeJoinManager (){
	
	closeTable(table);
	closeTable(table2);
	closeTable(table3);
    return RC_OK;
}

/************************* Nested Join ************************************/
extern RC nested_join(JoinInfo *j1,Schema *s1,JoinInfo *j2,Schema *s2,Schema *s3,RID *res)
{

	
	int a1=j1->attrNum;
	int a2=j2->attrNum;
	int i,j,x,z=0;
	RID rel1,rel2,rel3;
	Value *v1,*v2;
	
	Record *r1= (Record*) malloc(sizeof(Record));  // To store first relation
	Record *r2= (Record*) malloc(sizeof(Record));  // to store second relation
	Record *r3;				       // To store the result
  	/*openTable(table, t1);
  	mgmtinfo=table->mgmtData;*/
  	
  	//Set pointers on both relations to scan
  	
  	int tc1= mgmtinfo->Tuplescount;
  	rel1.page=mgmtinfo->FirstPage;
  	rel1.slot=0;
  	
  	int tc2= mgmtinfo2->Tuplescount;
  	rel2.page=mgmtinfo->FirstPage;
  	rel2.slot=0;
  	
  	// Set pointer to write the result to page file
  	rel3.page=mgmtinfo3->FirstPage;
  	rel3.slot=0;
	
  	printf("\nJoined tuples are: \n");
  	// Traversing
	for(i=0;i<tc1;i++)
	{
		//printf("\n Entering here");
	   getRecord(table,rel1,r1);
	   //printf("\n Getting record");
	   getAttr(r1,s1,a1,&v1);
	   //printf("\n Here Successfully");
	   
	   //printf("\n 2nd record %d",getRecord(table3,rel3,r3));
	   //printf("\n Ok then");
	   rel2.page=mgmtinfo2->FirstPage;
	   rel2.slot=0;
	   
	   // traverses relation 2
	   for(j=0;j<tc2;j++)
	   {
	   	
	   	getRecord(table2,rel2,r2);
	  	getAttr(r2,s2,a2,&v2);
	  	//printf("\n 2nd tuple");
	  	if( v2->dt==DT_INT)
	  	{
	  	        if(v2->v.intV == v1->v.intV)
	  	        {
	  	        	//printf("\n 2nd tuple twice");
	  	        	createRecord(&r3,s3);
	  	        	print(s1,r1,v1,r3,s3);
	  	        	x=s1->numAttr;	
	  	             	print2(s2,r2,v2,r3,s3,x,a2);
	  	             	insertRecord(table3,r3);
	  	             	res[z]=r3->id;
	  	             	z++;
         			printf("\n");
         			if(rel3.slot < (mgmtinfo2->maxSlots-1))
	  	       		{
	  	      			//printf("\n Increase slot");	
	  	      			rel3.slot++; 
	  	       		}
	  	       		else
	  	  		{
	  	     
					rel3.page++;
					rel3.slot=0;	  	       
	  	       		}			
         		}			          	 
	  	}
	  	if(rel2.slot < (mgmtinfo2->maxSlots-1))
	        {
		      	//printf("\n Increase slot");	
	  	      	rel2.slot++; 
	  	}
	  	else
	  	{
			rel2.page++;
			rel2.slot=0;	  	       
      	        }
	  	       
	  }	  	
	  if(rel1.slot < (mgmtinfo->maxSlots-1))
	  {
	       		//printf("\n Increasing slots for r1");
	  	      	rel1.slot++; 
 	   }
	   else
	   {       
			rel1.page++;
			rel1.slot=0;	  	       
           }
 	 }
	
	// Free all the allocated memory
	
	free(r1);
	free(r2);
	free(v1);
	free(v2);
		
	return RC_OK;
}		
	
/****************** Print the Schema **********************/
void print(Schema *s1,Record *r1,Value *v1,Record *r2,Schema *s2)
{
	int k;
	// Traverse the schema 
	for(k=0;k<s1->numAttr;k++)
        {
  	             	//printf("\n %d",);
	  	             	//printf("\n Ok very nice");
		getAttr(r1,s1,k,&v1);
		// Setting the attributes according to the datatypes
	  	switch (s1->dataTypes[k])
	  	{
            		case DT_STRING:
                		printf(" %s ",v1->v.stringV);
                		setAttr(r2,s2,k,v1);
                		break;
                		
            		case DT_INT:
               			 printf(" %d ",v1->v.intV);
               			 setAttr(r2,s2,k,v1);
                		break;
                			
            		case DT_FLOAT:
                		printf(" %f ",v1->v.floatV);
                		setAttr(r2,s2,k,v1);
                		break;
            			
            		case DT_BOOL:
                		//printf(" %f ",v1->v.floatV);
                		setAttr(r2,s2,k,v1);
                		break;
         	}
         }
}

/*************** Print 2 to print the second relation ************************/

void print2(Schema *s1,Record *r1,Value *v1,Record *r2,Schema *s2,int x,int a2)
{
	int k;
	for(k=0;k<s1->numAttr;k++)
	{
		getAttr(r1,s1,k,&v1);
	  	if(k!=a2)
	  	{
	  	    	switch (s1->dataTypes[k])
	  	       	{
            			case DT_STRING:
                			printf(" %s ",v1->v.stringV);
                			setAttr(r2,s2,x,v1);
                			x++;
                			break;
                		
            			case DT_INT:
               			 	printf(" %d ",v1->v.intV);
               			 	setAttr(r2,s2,x,v1);
               			 	x++;
                			break;
                			
            			case DT_FLOAT:
                			printf(" %f ",v1->v.floatV);
                			setAttr(r2,s2,x,v1);
                			x++;
                			break;
                		
                		case DT_BOOL:
                			//printf(" %f ",v1->v.floatV);
                			setAttr(r2,s2,x,v1);
                			x++;
                			break;
         		}
         	  }
         }
}

/**************** Union of two relations *******************/
 extern RC union_of(Schema *s1,Schema *s2)
{
	int i,j,flag=0,flag1=0;
	RID rel1;
  	RID rel2;
	Value *v1,*v2;
	// Check if same number of attributes
	if(s1->numAttr == s2->numAttr)
	{
		flag1=0;
		for(i=0;i<s1->numAttr;i++)
		{
			{
			// Check if the datatypes are matching
			 if(s1->dataTypes[i] != s2->dataTypes[i])
			 	flag1=1;
			 }
		}
	}
	else
	{
		return RC_UNION_NOT_POSSIBLE;
	}
	
	if(flag1==1)
	{
		return RC_UNION_NOT_POSSIBLE;
	}
	
	// Initialize the pointers
	Record *r1= (Record*) malloc(sizeof(Record));
	Record *r2= (Record*) malloc(sizeof(Record));
	int tc1= mgmtinfo->Tuplescount;
  	rel1.page=mgmtinfo->FirstPage;
  	rel1.slot=0;
  	
  	int tc2= mgmtinfo2->Tuplescount;
  	rel2.page=mgmtinfo->FirstPage;
  	rel2.slot=0;
  	
  	printf("\n Union tuples are: \n");
	// Compute the union and print on console
	for(i=0;i<tc1;i++)
	{
		
	   getRecord(table,rel1,r1);
	   getAttr(r1,s1,0,&v1);
	   rel2.page=mgmtinfo2->FirstPage;
	   rel2.slot=0;
	   flag=0;
	   for(j=0;j<tc2;j++)
	   {
	   	
	   	getRecord(table2,rel2,r2);
	  	getAttr(r2,s2,0,&v2);
	  	//printf("\n 2nd tuple");
	  	if( v2->dt==DT_INT)
	  	{
	  	        if(v2->v.intV == v1->v.intV)
	  	        {
	  	        	flag=1;
	  	        	//printf("\n 2nd tuple twice");
	  	        		printu(s1,r1,v1);	
	  	             		printf("\n");
	  	             		printu(s2,r2,v2);
         				printf("\n");		

         			}	  	            
	  	       	 
	  	       }
	  	       if(rel2.slot < (mgmtinfo2->maxSlots-1))
	  	       {
	  	      		//printf("\n Increase slot");	
	  	      		rel2.slot++; 
	  	       }
	  	       else
	  	       {
	  	       
					rel2.page++;
					rel2.slot=0;	  	       
	  	       }	       
	  	}
	  	if(flag==0)
	  	{
	  
	  		printu(s1,r1,v1);
			printf("\n");  		
	  	
	  	}
	  	if(rel1.slot < (mgmtinfo->maxSlots-1))
	  	{
	  	 	//printf("\n Increasing slots for r1");
	  	      	rel1.slot++; 
	  	}
	  	else
	  	{       
			rel1.page++;
			rel1.slot=0;	  	       
	  	}
	  }
	// For the second half of the relation
	rel1.page=mgmtinfo->FirstPage;
  	rel1.slot=0;
  	rel2.page=mgmtinfo->FirstPage;
  	rel2.slot=0;
	for(i=0;i<tc2;i++)
	{
		
	   getRecord(table2,rel2,r2);
	   getAttr(r2,s2,0,&v2);
	   rel1.page=mgmtinfo->FirstPage;
	   rel1.slot=0;
	   flag=0;
	   for(j=0;j<tc1;j++)
	   {
	   	
	   	getRecord(table,rel1,r1);
	  	getAttr(r1,s1,0,&v1);
	  	//printf("\n 2nd tuple");
	  	if( v2->dt==DT_INT)
	  	{
	  	        if(v2->v.intV == v1->v.intV)
	  	        {
	  	        	flag=1;
	  	        	
         		}			         
	  	  }
	  	  if(rel1.slot < (mgmtinfo->maxSlots-1))
	  	  {
	     		//printf("\n Increasing slots for r1");
	  	      	rel1.slot++; 
	  	   }
	  	   else
	  	   {    
			rel1.page++;
			rel1.slot=0;	  	       
       	           }       
	     }
	  	
	     if(flag==0)
	     {
	  	printu(s2,r2,v2);
		printf("\n");  		
	  	
	      }
	      if(rel2.slot < (mgmtinfo2->maxSlots-1))
	      {
	       	//printf("\n Increase slot");	
	  	rel2.slot++; 
	      }
	      else
	      {
	  	 rel2.page++;
		rel2.slot=0;	  	       
	      }
	  	
	  }

	
	free(r1);
	free(r2);
	free(v1);
	free(v2);
	return RC_OK;
}	

/**************** Print all attributes *******************/

void printu(Schema *s1,Record *r1,Value *v1)
{
	int k;
	for(k=0;k<s1->numAttr;k++)
	{
	       	getAttr(r1,s1,k,&v1);
	  	           	
	  	switch (s1->dataTypes[k])
	  	{
            		case DT_STRING:
                		printf(" %s ",v1->v.stringV);
                		break;
                		
            		case DT_INT:
               			printf(" %d ",v1->v.intV);	 
                		break;
                			
            		case DT_FLOAT:
                		printf(" %f ",v1->v.floatV);	
                		break;
            	
            		case DT_BOOL:    		
                		break;

         	 }
         			
         }
}
