# Makefile for QuoteOptions
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=$(MYSQL_FLAGS) $(CURL_FLAGS)

QuoteOptions: QuoteOptions.o get_exp_dates.o
	$(CC) $(CFLAGS) QuoteOptions.o get_exp_dates.o $(LDFLAGS) -o QuoteOptions

QuoteOptions.o: QuoteOptions.c QuoteOptions.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) QuoteOptions.c

get_exp_dates.o: get_exp_dates.c QuoteOptions.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) get_exp_dates.c

install:
	mv ./QuoteOptions /Finance/bin/C/

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
