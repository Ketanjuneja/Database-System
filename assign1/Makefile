

dberror.o: dberror.c
	cc -o dberror.o -c dberror.c

storage_mgr.o: storage_mgr.c
	cc -o storage_mgr.o -c storage_mgr.c

test_assign1_1.o: test_assign1_1.c
	cc -o test1.o -c test_assign1_1.c

test_assign1_2.o: test_assign1_2.c
	cc -o test2.o -c test2.c

a1: test1.o storage_mgr.o dberror.o
		cc -o a1 test_assign1_1.o storage_mgr.o dberror.o -I.

a2: test2.o storage_mgr.o dberror.o
		cc -o a2 test_assign1_2.o storage_mgr.o dberror.o -I.

