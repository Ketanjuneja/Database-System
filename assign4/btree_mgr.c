#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "storage_mgr.h"

#include "btree_mgr.h"
#include "tables.h"
#define size_x 50
#define size_n 5

/**************** Structure for the B+ tree ******************/

typedef struct Bptree{
	int keyi; 
	float keyf;
	char *keys;
	DataType dt;
	int flag;
	struct RID rid;
	
}Bptree;

SM_FileHandle *fh;
int node_count=0,next_node=0;
Bptree **btree; 

/******************** Initialize Index MAnager ******************/
/* Allocate the memory to every btree structure */
RC initIndexManager (void *mgmtData)
{

	btree=(Bptree **)malloc(size_x*sizeof(Bptree* ));
	return RC_OK;
}

/********************** ShutDown Index MAnager ******************/
/* free memory for particular b tree */
RC shutdownIndexManager ()
{
	
	free(btree);
	return RC_OK;
}

/*********************** Create B+ Tree ***********************/
/* Initialize the BTreeHandle 
   Initialize the variables for BTree datastructure
*/
RC createBtree (char *idxId, DataType keyType, int n)
{
	BTreeHandle *data=(BTreeHandle *)malloc(sizeof(BTreeHandle)*size_n);
	/* 
	bptree root =(Bptree*) malloc(sizeof(BPtree));
	root=NULL;
	*/
	//printf("\n File Name %s",idxId);
	data->keyType=keyType;
	createPageFile(idxId);
	return RC_OK;
}

/********************** Open B+ Tree Page *********************/
/* Accepts Tree Handle and idxId 
   Opens the corresponding page file
*/
RC openBtree (BTreeHandle **tree, char *idxId)
{
	(*tree)=(BTreeHandle *)malloc(sizeof(tree)*size_n);
	int length=strlen(idxId);
	(*tree)->idxId=(char *)malloc(sizeof(char)*(length+1));
	(*tree)->idxId=idxId;
	//openPageFile(idxId,&fh);
	return RC_OK;
}

/*********************** Close B+ Tree Page ******************/
RC closeBtree (BTreeHandle *tree)
{
	free(tree);
	//closePageFile(&fh);
	return RC_OK;
}

/*********************** Delete B+ Tree ********************/
/* Destroys the Page file corresponding to B+ Tree */
RC deleteBtree (char *idxId)
{
	node_count=0;
	next_node=0;
	//destroyPageFile(idxId);
	return RC_OK;
}

/*********************** Get Number of Nodes **************/
RC getNumNodes (BTreeHandle *tree, int *result)
{

	int num,i=1,j,count=0;
		
	while(i<node_count)	 // Traverse the nodes
	{
		j=i-1;
		while(j>-1)
		{
		          /* printf("\directly return");
		              count =i;
		              */
			if(btree[i]->rid.page==btree[j]->rid.page)
			{
				count++;	// Increment the count 
				//printf("\n Here the page is %d",btree[i]->rid.page);
				break;
			}
			j--;
		}
		i++;
		

	}
	// Store matched number of nodes in num
	
	num=count;  
	*result=(node_count-num);
	
	return RC_OK;
}

/********************** Get the Number of Entries ******************/
RC getNumEntries (BTreeHandle *tree, int *result)
{
	*result=node_count;
	return RC_OK;
}

/********************** Get the type of Key ***********************/
RC getKeyType (BTreeHandle *tree, DataType *result)
{	
	//result=btree[0]->dt;
	return RC_OK;
}

/********************** Search key ********************************/
RC findKey (BTreeHandle *tree, Value *key, RID *result)
{
	int i=0,search=0;
	
	while(i<node_count && search==0)
	{
             if(btree[i]->dt==key->dt && key->dt==DT_INT)
             {
             
             	if(btree[i]->keyi==key->v.intV)
             	 search=1;
             }	 
		
		if(btree[i]->dt==key->dt && key->dt==DT_FLOAT)
             {
             
             	if(btree[i]->keyf==key->v.floatV)
             	 search=1;
             }
             if(btree[i]->dt==key->dt && key->dt==DT_STRING)
             {
             
             	if(strcmp(btree[i]->keys,key->v.stringV)==0)
             	 search=1;
             }
               i++;
            }
	
	// If the key is found	
	if(search==1)
	{	i--;
		result->page=btree[i]->rid.page;
		result->slot=btree[i]->rid.slot;
		return RC_OK;
	}
	else{
		return RC_IM_KEY_NOT_FOUND;
	}

	
	

}

/*********************************** Insert key in the B+ tree *****************/

RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
	btree[node_count]=(Bptree *)malloc(sizeof(Bptree));
	int i;
	RID *x=(RID*)malloc(sizeof(RID));
	
	
	if(findKey(tree,key,x)==RC_OK)
	return RC_IM_KEY_ALREADY_EXISTS;
	
	// If key does not already exists
	else
	{
		
		if(DT_INT==key->dt)
		{
			btree[node_count]->dt=key->dt;
			btree[node_count]->keyi=key->v.intV;
		}
		if(DT_STRING==key->dt)
		{
			btree[node_count]->dt=key->dt;
			strcpy(btree[node_count]->keys,key->v.stringV);
		}
			
		if(DT_FLOAT==key->dt)
		{
			btree[node_count]->dt=key->dt;
			btree[node_count]->keyf=key->v.floatV;
		}
			
		
		btree[node_count]->rid.page=rid.page;
		btree[node_count]->rid.slot=rid.slot;
		node_count++;

		return RC_OK;
	}


}

/************************* Delete the key from the B+ tree ****************/
/* Accept the tree handle and the key
* Search for the key in the tree
* Delete the key and adjust the B+ tree 
***************************************************************************/
RC deleteKey (BTreeHandle *tree, Value *key)
{
	int i=0,j,search=0;
	int temp=0;

	while(i<node_count && search==0)
	{
             if(btree[i]->dt==key->dt && key->dt==DT_INT)
             {
             
             	if(btree[i]->keyi==key->v.intV)
             		search=1;
             }	 
		
	     if(btree[i]->dt==key->dt && key->dt==DT_FLOAT)
             {
             	if(btree[i]->keyf==key->v.floatV)
             		 search=1;
             }
             
             if(btree[i]->dt==key->dt && key->dt==DT_STRING)
             {
             
             	if(strcmp(btree[i]->keys,key->v.stringV)==0)
             	 search=1;
             }
               i++;
        }

	
		if(search==1)
		{
		i--;
		temp=i+1;
		j=i;
		while(j<node_count&&temp<node_count)
		{
			
			if(btree[temp]->dt == DT_INT)
			{
				btree[j]->dt=btree[temp]->dt;
				btree[j]->keyi=btree[temp]->keyi;
				
			}
			else if(btree[temp]->dt == DT_STRING)
			{
				btree[j]->dt=btree[temp]->dt;
				strcpy(btree[j]->keys,btree[temp]->keys);
				
			}
			else if(btree[temp]->dt == DT_FLOAT)
			{
				btree[j]->dt=btree[temp]->dt;
				btree[j]->keyf=btree[temp]->keyf;
			}
			
			btree[j]->rid.page=btree[temp]->rid.page;
			btree[j]->rid.slot=btree[temp]->rid.slot;
			j++;
			temp++;
		}
		
		node_count--;
		free(btree[j]);
	    	return RC_OK;
	}
	else
	{
		return RC_IM_KEY_NOT_FOUND;
	}

}

/********************************* Open Scan for Tree ****************************/
RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
//	handle->tree=tree;
	return RC_OK;
}

/********************************* Swap b+ Trees to maintain the condition for the B+ tree *****/ 
void swap(Bptree **btree,int a,int b)
{
	Value valtemp;
	RID ridtemp;
	int c;
	valtemp.dt=btree[a]->dt;
	valtemp.v.intV=btree[a]->keyi;
	ridtemp.page=btree[a]->rid.page;
	ridtemp.slot=btree[a]->rid.slot;

	btree[a]->dt=btree[b]->dt;
	btree[a]->keyi=btree[b]->keyi;
	btree[a]->rid.page=btree[b]->rid.page;
	btree[a]->rid.slot=btree[b]->rid.slot;

	btree[b]->dt=valtemp.dt;
	btree[b]->keyi=valtemp.v.intV;
	btree[b]->rid.page=ridtemp.page;
	btree[b]->rid.slot=ridtemp.slot;

	while(a<node_count)
	{
		b = a; 
		b = b + 1; 
		c = a;
		a++;
	}

}

RC nextEntry (BT_ScanHandle *handle, RID *result)
{
	//sort
	int i,k,position;
	i=0;
	while(i<node_count-1)
	{
		position=i;
		k = i + 1; 
		while(k<node_count)
		{
			if(btree[k]->keyi < btree[position]->keyi)
			{
				position=k;
			}
			k++;
		}
		swap(btree,i,position);
		i++;
	}
	
	while(k<node_count)
	{
		position = k; 
		k = k + 1; 
		int nc = node_count;
		nc++;
	}
	
	if(next_node<node_count)
	{
		result->page=btree[next_node]->rid.page;
		result->slot=btree[next_node]->rid.slot;
		next_node++;
		return RC_OK;
	}
	else
	{
		return RC_IM_NO_MORE_ENTRIES;
	}

}

/**************************** Close the B+ tree Scan process ********************************/
RC closeTreeScan (BT_ScanHandle *handle)
{
	free(handle);

	return RC_OK;
}

