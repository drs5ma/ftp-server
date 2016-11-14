all: server2.c
	gcc -o ftp server2.c
clean:
	$(RM) fat
	$(RM) drs5ma.tar
tar:
	tar cfvz drs5ma.tar server2.c Makefile 


