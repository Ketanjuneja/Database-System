Problem Statement:
===================

The goal is to implement a B+-tree index. The index should be backed up by a page file and pages of the index should be 
accessed through buffer manager.Each node should occupy one page.However, for debugging purposes code should support trees 
with a smaller fan-out and still let each node occupy a full page.A B+-tree stores pointer to records (the RID introduced 
in the last assignment) index by a keys of a given datatype. code should support DT_INT (integer) keys (see optional 
extensions). Pointers to intermediate nodes should be represented by the page number of the page the node is stored in. 
To make testing easier your implementation should follow the conventions stated below.

Leaf Split: 
In case a leaf node need to be split during insertion and n is even, the left node should get the extra key. E.g, if n = 2 
and the key 4 should be inserted into a node [1,5], then the resulting nodes should be [1,4] and [5]. For odd values of n 
we can always evenly split the keys between the two nodes. In both cases the value inserted into the parent is the 
smallest value of the right node.

Non-Leaf Split: 
In case a non-leaf node needs to be split and n is odd, we cannot split the node evenly (one of the new nodes will have 
one more key). In this case the "middle" value inserted into the parent should be taken from the right node. E.g., if 
n = 3 and we have to split a non-leaf node [1,3,4,5], the resulting nodes would be [1,3] and [5]. The value inserted into 
the parent would be 4.

Leaf Underflow: In case of a leaf underflow, code should first try to redistribute values from a sibling and only if this 
fails merge the node with one of its siblings. Both approaches should prefer the left sibling. E.g., if we can borrow values
from both the left and right sibling, you should borrow from the left one.

Running the Test Cases:
=========================

Compile : make a1
Run : ./a1

To revert:
On terminal : make clean


Logic:
======

Index Manager Functions:
-------------------------

1. initIndexManager:
   This function Initializes index manager.

2. shutdownIndexManager:
   a. shuts down index manager.
   b. frees all acquired resources.

B+-tree Functions:
------------------

1. createBtree Function:
   This function creates a B tree

2. deleteBtree Function:
   a. This function deletes the B tree
   b. removes corresponding pagefiles.

3. openBtree Function:
   This function opens B tree

4. closeBtree Function
   a. This function closes B tree
   b. while doing so index manager ensures that all new or modified pages are flushed back to disk.

Key Functions:
--------------

1. findKey Function:
   a. Returns RID for the entry with the search key in the B tree
   b. if key does not exist it returns RC_IM_KEY_NOT_FOUND.

2. insertKey Function:
   a. inserts a new key and record pair pointer into the index.
   b. RC_IM_KEY_ALREADY_EXISTS is returned when the key is already present in the tree.

3. deleteKey Function:
   a. Removes a key from the index.
   b. RC_IM_KEY_NOT_FOUND is returned when the key is not found.

Scan Functions:
----------------
1. openTreeScan Function:
   This function opens the B tree for scanning

2. nextEntry Function
   This function reads next entry in the B tree

3. closeTreeScan Function
   Close B Tree after scanning.

Access Information Functions:
-----------------------------

1. getNumNodes Function:
   Total number of nodes in a tree is calculated in this function.

2. getNumEntries Function:
   This function calculates the total number of entries in B tree.

3. getKeyType Function:
   keyType is returned by this function.

Debug Functions:
----------------

1. printTree Function:
   This function creates a string representation of the B tree.


Contributions:
==============

Lasya Ramesh:
1. Index Functions
2. Access Information Functions

Ketan Kumar Juneja:
1. B tree functions
2. Key functions

Ketan Aswani:
1. Debug Functions
2. Scan Functions




