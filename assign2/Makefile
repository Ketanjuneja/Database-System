

dberror.o: dberror.c
	cc -o dberror.o -c dberror.c -g -Wall

storage_mgr.o: storage_mgr.c
	cc -o storage_mgr.o -c storage_mgr.c -g -Wall

buffer_mgr.o: buffer_mgr.c
	cc -o buffer_mgr.o -c buffer_mgr.c -g -Wall
	
buffer_mgr_stat.o: buffer_mgr_stat.c
	cc -o buffer_mgr_stat.o -c buffer_mgr_stat.c -g -Wall


test_assign2_1.o: test_assign2_1.c
	cc -o test_assign2_1.o -c test_assign2_1.c -g -Wall

a1: test_assign2_1.o buffer_mgr.o buffer_mgr_stat.c storage_mgr.o dberror.o
	cc -o a1 test_assign2_1.o buffer_mgr.o buffer_mgr_stat.c storage_mgr.o dberror.o -I. -g -Wall

clean: test_assign2_1.o buffer_mgr.o buffer_mgr_stat.o storage_mgr.o dberror.o
	rm test_assign2_1.o buffer_mgr.o buffer_mgr_stat.o storage_mgr.o dberror.o
