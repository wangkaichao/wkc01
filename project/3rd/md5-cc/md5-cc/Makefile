
SHELL=/bin/sh
CC=g++ -fPIC

md5:		driver.o md5.o
	$(CC) -o md5 driver.o md5.o
	$(AR) rcs libmd5cc.a $^

driver.o:	driver.cc
	$(CC) -c driver.cc

md5.o:		md5.hh md5.cc
	$(CC) -c md5.cc

clean:
	-rm *.o md5 libmd5cc.a
