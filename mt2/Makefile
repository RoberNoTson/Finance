# Makefile for
# mt2 
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=-lpthread $(MYSQL_FLAGS) $(CURL_FLAGS)

mt2: mt2.o usage.o do_output.o do_updates.o  ../Includes/BidAsk.inc ../Includes/print_error.inc ../Includes/EnhPivot.inc ../Includes/Chandelier.inc ../Includes/Safe.inc
	$(CC) $(CFLAGS) mt2.o usage.o do_output.o do_updates.o $(LDFLAGS) -o mt2

mt2.o: mt2.c mt2.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) mt2.c

usage.o: usage.c mt2.h
	$(CC) -c $(MYSQL_FLAGS) usage.c

do_output.o: do_output.c mt2.h
	$(CC) -c $(MYSQL_FLAGS) do_output.c

do_updates.o: do_updates.c mt2.h
	$(CC) -c $(MYSQL_FLAGS) do_updates.c
install:
	mv ./mt2 /Finance/bin/C/

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
