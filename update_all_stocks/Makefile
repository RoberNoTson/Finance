# Makefile for
# update_all_stocks
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs_r`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=$(MYSQL_FLAGS) $(CURL_FLAGS)

update_all_stocks: update_all_stocks.o usage.o time_check.o parse_update.o
	$(CC) $(CFLAGS) update_all_stocks.o usage.o time_check.o parse_update.o $(LDFLAGS) -o update_all_stocks

update_all_stocks.o: update_all_stocks.c update_all_stocks.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) update_all_stocks.c

usage.o: usage.c update_all_stocks.h
	$(CC) -c $(MYSQL_FLAGS) usage.c

time_check.o: time_check.c update_all_stocks.h
	$(CC) -c $(MYSQL_FLAGS) time_check.c

parse_update.o: parse_update.c update_all_stocks.h
	$(CC) -c $(MYSQL_FLAGS) parse_update.c

install:
	mv ./update_all_stocks /Finance/bin/C/update_all_stocks

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
