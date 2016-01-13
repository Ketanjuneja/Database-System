Optional Assignment:
=====================

The goal of this assignment is to implement a standard operator algorithm
for Union and Nested Loop join on top of your record manager. 

Running the Test Cases:
========================

Compile : make a1
Run : ./a1

To revert:
On terminal : make clean

Logic:
======

We have implemented two of the given set of operations in this assignment.
 
Here we consider two relations. Let us call them Relation 1 and Relation 2

Relation 1 has the following tuple entries : 

--------------------------------------------------
|	a	|	b	|	c	|
--------------------------------------------------
|	1	|     aaaa	|	3	|	
|	2	|     bbbb	|	2	|
|	3	|     cccc	|	1	|
|	4	|     dddd	|	3	|
|	5	|     eeee	|	5	|
|	6	|     ffff	|	1	|
|	7	|     gggg	|	3	|
|	8	|     hhhh	|	3	|
|	9	|     iiii	|	2	|
-------------------------------------------------

And similarly, Relation 2 has the following tuples: 

--------------------------------------------------
|	a	|	d	|	e	|
--------------------------------------------------
|	5	|     nnnn	|	5	|
|	6	|     oooo	|	1	|
|	7	|     pppp	|	3	|
|	8	|     qqqq	|	3	|
|	9	|     rrrr	|	2	|
|	10	|     qqqq	|	8	|	
--------------------------------------------------

We have performed TWO operations on the above data(test data), that can be performed on any other relations:

1. Nested Loop Join :  

The Nested loop join joins the given relations on matching a particular common attribute and outputs the tuples having the same attribute values in both relations.
The expected result of the Nested Loop Join on the two relations given above is as follows: 

---------------------------------------------------------------------------------
|	a	|	b	|	c	|	d	|	e	|
---------------------------------------------------------------------------------
|	5	|     eeee      |	5	|      nnnn     |	5	|
|	6	|     ffff	|	1	|      oooo	|	1	|
|	7	|     gggg	|	3	|      pppp	|	3	|
|	8	|     hhhh	|	3	|      qqqq	|	3	|
|	9	|     iiii	|	2	|      rrrr	|       2	|
---------------------------------------------------------------------------------

The result obtained from our code has been written back to a page file named "test_table_r3.txt".
We have implemented the test cases to assert the result of the code with the expected output. Besides this, the results have also been printed on the console. 

2. Union:

In this operation, we check the number of attributes and the type of attributes in the two relations. If they are same then we can perform the union of the two.
Here the two relations have similar attributes. The expected result of the Union on two relations given is as follows:

-------------------------------------------------
|	a	|	b	|	c	|
-------------------------------------------------
|	1	|     aaaa	|	3	|
|	2	|     bbbb	|	2	|
|	3	|     cccc	|	1	|
|	4	|     dddd	|	3	|
|	5	|     eeee      |	5	|
|	5	|     nnnn	|	5	|
|	6	|     ffff	|	1	|
|	6	|     oooo	|	1	|
|	7	|     gggg	|	3	|
|	7	|     pppp	|	3	|
|	8	|     hhhh	|	3	|
|	8	|     qqqq	|	3	|
|	9	|     iiii	|	2	|
|	9	|     rrrr	|	2	|
|	10	|     qqqq	|	8	|	
-------------------------------------------------

The results of the union operations has been printed on the console and verified with the above expected output. So, if the client does not want to write it back to
the disk then this option can be used to access the resulting table.


Implementation:
===============

Data Structures used :
-------------------------

(1) JoinInfo: Stores information of join attribute.
	
Functions used:
-----------------

(1) initJoinManager(): Initializes the join manager.

(2) shutdownJoinManager(): closes the Join task.

(3) OpenJoinManager(Schema *, char *, Schema *, char *, Schema *, char *) : Accepts the three schema information. Two, on which the join will be performed and one to 
    store the result of the join.

(4) closeJoinManager(): Deallocates all the variables used for scanning and union operations.

(5) nested_join(JoinInfo *, Schema* , JoinInfo *, Schema* , Schema *, RID *): The nested loop join is implemented in this function. For each tuple in relation R1, we 
    scan through each tuple in relation R2. If the join condition matches then the output is written to a page file on the disk (test_table_r3.txt). These values are 
    matched with the expected values in the test cases. 

(6) union_of(Schema *, Schema *): This function accepts the two relations R1 and R2 and then checks the datatypes and number of attributes for each relation. If these
    conditions are passed the union of the relations is computed and the resultant table/relation is displayed on the console. This covers the case if the client 
    only wants to view the resultant relation and not write it back to the disk.


Test Cases
===========

(1) Both relations are created and the tuples are inserted. 

(2) A third schema to store the result of the join is initialized.

(3) The nested loop join is performed by the join manager and the result is asserted with the above expected schema.

(4) Union operation is performed and the result is displayed on the console.








