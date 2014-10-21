# Makefile for daily_div_list
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=$(MYSQL_FLAGS) $(CURL_FLAGS)

daily_div_list: daily_div_list.o
	$(CC) $(CFLAGS) daily_div_list.o $(LDFLAGS) -o daily_div_list

daily_div_list.o: daily_div_list.c daily_div_list.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) daily_div_list.c

install:
	mv ./daily_div_list /Finance/bin/C/

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
