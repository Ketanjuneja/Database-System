

Problem Statement:  
This assignment is about creating a record manager. The record manager handles tables with a fixed schema. Clients can insert records, delete records, update records, and scan through the records in a table. A scan is associated with a search condition and only returns records that match the search condition. Each table should be stored in a separate page file and your record manager should access the pages of the file through the buffer manager implemented in the last assignment.

Running the script:
With default test cases:
Compile: make a1
Run:./a1

Compile: make expr
Run: ./expr
To revert:
On terminal: make clean

Logic:
Few attributes were defined and few are required by the record manager. Structure of such attributes is implemented called tablemgmt. This structure has the following attributes:
·	FirstPage - First Record’s page number in the table.
·	LastPage  - Last Record’s Page number in the table.
·	Tuplescount       - number of tuples present in a table
·	slotSize        - size of the slot holding record.
·	maxSlots        - maximum number of slots.
·	tstone_root     - tstone list’s head node.

Few attributes are required to perform scan to retrieve all tuples from the table that fulfill certain conditions. A structure of such attributes called recordInfon has been implemented. It has the following attributes:
 a. cond     - condition given by the client.
b. Slot  - record’s current slot.
c. Page  - record’s current page.
Few attributes are required to track a record that is deleted. A Strusture recordInfo tNode has been implemented for the same. The attributes are:
Page , slot   - page id, slot id and tombstone id of a record.
next - next node in the linked list.
Table and Record Manager Functions:
initRecordManager:
a. This function initializes the set up values and starts the record manager,
b. Value 1 is passed as argument for primary key constraint

shutdownRecordManager:
 Record manager’s assigned resources are freed.

createTable: 
a.	Creates table with the given name and If the given name for the table already exists, an error must be thrown.
b.	 it writes the tablemgmt on the page 0 and schema on page 1 when new file is created.
c.	The FirstPage is set to page 1.

openTable:
a. Checks for the existence of the file. If the file does not exist it throws an error. 
b. opens the file with given table name.
c. initialize buffer manager with given filename.
d. Reads page 0 , loads tablemgmt and schema into memory.

closeTable:
1.	Checks for the availability of file. If file does not exist throws error. 
2.	Closes the file, the buffermanager of a given file, frees all the resources assigned with table.
deleteTable:
a. Checks for the availability of the table, if table does not exist throws an error.
b. It deletes the file, and destroys the buffer manager associated with it.

getTuplescount:
a. Tuplescount is a part of tablemgmt. This is updated every time a record in inserted or deleted.
b. Stored on page 0 when written to file and loaded to memory.

Record Functions :
insertRecord:
a. Checks to see if there are any RID's in the tstone list. If any, one of the RID’s are used and the record's attributes are updated accordingly.
b. A new page is created If the new slot's location is equal to the maximum number of slots for a page, the current page is used otherwise. The record's values for page and slot are updated accordingly.
c. To get the record in the proper format serializeRecord is used
d. To update the buffer pool The functions pinPage, markDirty, unpinPage, and forcePage are then used,    the resulting table is written to file by increasing the number of tuples.

deleteRecord:
a. if the tstone list is empty. it creates a new tNode, updates its contents with the values from RID, and adds it to the tstone list.
b. tstone_iter is used to go to the end of the list and add a new tNode (with RID's contents) there if the list is not empty.
c. The number of tuples are reduced and the resulting table is written to file.

updateRecord:
a. To get the record in the proper format serializeRecord is used.
b. To update the buffer pool the functions pinPage, markDirty, unpinPage, and forcePage are used, the resulting table is written to file.

getRecord:
a. This uses RID value to return a record to the user.
b. Checks to see if RID is not in the tstone list. if it is a valid record, then checksif the tuple number is greater than the number of tuples in the table if so then an error is thrown.
c. To update the buffer pool then, pinPage and unpinPage is used.
d. To obtain a valid record from the record string which was retrieved, deserializeRecord is used. Data in the record is then updated accordingly.

Scan Functions:
startScan:
a. initializes the RM_ScanHandle that is passed as an argument to it.
b. initializes a node storing the information regarding the record that is to be searched and the condition to be evaluated.
c. The node initialed is assigned to scan->mgmtData.

next:
a. fetches record as per the page and slot id.
b. checks tombstone id for a deleted record.
c. If the record is a deleted one then it checks for record to see if it is the last record through slot number.
d. Slot id is set to be 0 to start  next scan from the begining of the next page In-case of the last record
e. If the record is not the last one, the slot number is increased by one to proceed to the next record.
f. scan mgmtData has updated record id parameters and next function is called.
g. Once tombstone parameters of the record is verified, the given condition is evaluated to check if the record is the one required by the client.
h. The next function is called again with the updated record id parameters If the record fetched is not the required one
i. it returns RC_RM_NO_MORE_TUPLES once the scan is complete and RC_OK otherwise if no error occurs.
j. Calls to this function returns next tuple that fulfills scan condition.


closeScan:
free scan handler  indicating to record manager that all associated resources are cleaned up.

Schema Functions:
getRecordSize:
a. Returns the size of the record.
b. Counts the size of the record based on the schema. The datatype of each attribute is taken into account for this calculation.

createSchema:
a. creates the schema object and assigns memory.
b. Number of attributes, datatypes and size (in case of DT_STRING) is stored.

freeSchema:
Frees all memory assigned to schema object:
   a. DataType
   b. AttributeNames
   c. AttributeSize

Attribute Functions
createRecord:
Memory allocation happens for a record and record data for creating a new record. This is  as per the schema.

freeRecord:
free the memory space occupied by record.

getAttr:
a. Allocates space to the value data structure, this is where the attribute values are to be fetched.
b. attrOffset function is called to get offset value for different attributes according to attribute numbers.
c. Attribute data is assigned to the value pointer according to different data types. This fetches attribute values of a record.

setAttr:
a. This function calls the attrOffset function to get the offset value of different attributes according to the attribute numbers.
b. Attribute values are set with values provided by the client as per the attributes datatype.
c. sets attribute values of a record.


Helper Functions:
tablemgmtToFile :
a. writes the tablenInfo to the file.
b. The tablemgmt is written on page 0. The keyinfo is written on the page 2.


deserializeRecord :
Reads record from the table file, parses it, and returns a record data object.

deserializeSchema :
Reads the schema from the table file, parses it, and returns a schema object.

slotSize :
calculates slot size required to write one record on the file based on the serializeRecord function.

tablemgmtToStr and strToTablemgmt :
Converts tablemgmt to a string to write to file, and converts the read data from the file to tablemgmt object.

keyInfoToStr and strToKeyInfo :
Converts the keyAttributeInfo to a string to write on the file, and converts the read data from the file
to keyList.

Additional Features:
Tombstones:
a. tablemgmt->tstone_root consists of Tombstones. This list consists of tNodes, each of them contains a RID and a pointer to the next tNode. This list is used in the record functions.
b. RID has another attribute of type Boolean, tstone. This is true for the given RID if that RID is a tombstone. This is helpful for scan functions to check to see which values need to be skipped when traversing records easily. The RID struct is altered in tables.h.

Contribution:
1.Ketan Aswani:
  Record Functions.
  Tombstone Implementation.
  Checking valgrind/memory leak issues.
  Extra valgrind/memory leaks in test files.

2. Lasya Ramesh:
  Attribute Functions. 
  Schema functions.

3. Ketankumar Juneja:
   Data structure and design.
   Table and manager functions.
   Scan Functions.  
   Helper Functions.
