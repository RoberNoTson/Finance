# Makefile for backpop
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=$(MYSQL_FLAGS) $(CURL_FLAGS)

backpop: backpop.o usage.o ParseData.o get_history.o
	$(CC) $(CFLAGS) backpop.o usage.o ParseData.o get_history.o $(LDFLAGS) -o backpop

backpop.o: backpop.c backpop.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) backpop.c

usage.o: usage.c backpop.h
	$(CC) -c $(MYSQL_FLAGS) usage.c

ParseData.o: ParseData.c backpop.h
	$(CC) -c $(MYSQL_FLAGS) ParseData.c

get_history.o: get_history.c backpop.h
	$(CC) -c $(MYSQL_FLAGS) get_history.c

install:
	mv ./backpop /Finance/bin/C/

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
