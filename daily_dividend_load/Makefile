# Makefile for daily_dividend_load
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=$(MYSQL_FLAGS) $(CURL_FLAGS)

daily_dividend_load: daily_dividend_load.o xlate_date.o
	$(CC) $(CFLAGS) daily_dividend_load.o xlate_date.o $(LDFLAGS) -o daily_dividend_load

daily_dividend_load.o: daily_dividend_load.c daily_dividend_load.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) daily_dividend_load.c

xlate_date.o: xlate_date.c daily_dividend_load.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) xlate_date.c

install:
	mv ./daily_dividend_load /Finance/bin/C/

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
