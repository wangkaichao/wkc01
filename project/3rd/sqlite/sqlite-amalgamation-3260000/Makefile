#Makefile

CC := gcc
AR := ar
LD := ld
STRIP := strip

CSOURCE := $(shell find ./ -name "*.c")
HSOURCE := $(shell find ./ -name "*.h")
OBJECTS := $(patsubst %.c, %.o, $(CSOURCE))

LDLIBS := -lpthread -ldl

all: sqlite3 libsqlite3.so libsqlite3.a

sqlite3:
	$(CC) $(CSOURCE) $(LDLIBS) -o $@
	@#gcc shell.c sqlite3.c -lpthread -ldl -o sqlite3
	@#$(STRIP) $@

libsqlite3.so:
	$(CC) sqlite3.c $(LDLIBS) -fPIC -shared -o $@
	@#gcc sqlite3.c -lpthread -ldl -fPIC -shared -o libsqlite3.so

libsqlite3.a: 
	$(CC) -c sqlite3.c $(LDLIBS) -o sqlite3.o
	$(AR) -r $@ sqlite3.o
	@#gcc -c sqlite3.c -lpthread -ldl -o sqlite3.o
	@#ar -r libsqlite3.a sqlite3.o

.PHONY:clean
clean:
	rm sqlite3 libsqlite3.so libsqlite3.a sqlite3.o
