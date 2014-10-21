# Makefile for
# my-trades 
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=-lpthread $(MYSQL_FLAGS) $(CURL_FLAGS)

my-trades: my-trades.o usage.o  delete_bad.o BullishOB.o BullishKR.o TrendUp.o RSI_check.o do_output.o MACD_check.o HV_check.o ../Includes/Chandelier.inc ../Includes/BidAsk.inc  ../Includes/print_error.inc ../Includes/valid_date.inc ../Includes/holiday_check.inc ../Includes/Safe.inc  ../Includes/ParseData.inc
	$(CC) $(CFLAGS) my-trades.o usage.o  delete_bad.o BullishOB.o BullishKR.o TrendUp.o RSI_check.o do_output.o MACD_check.o HV_check.o $(LDFLAGS) -o my-trades

my-trades.o: my-trades.c my-trades.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) my-trades.c

usage.o: usage.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) usage.c

delete_bad.o: delete_bad.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) delete_bad.c

BullishOB.o: BullishOB.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) BullishOB.c

BullishKR.o: BullishKR.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) BullishKR.c

TrendUp.o: TrendUp.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) TrendUp.c

RSI_check.o: RSI_check.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) RSI_check.c

MACD_check.o: MACD_check.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) MACD_check.c

HV_check.o: HV_check.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) HV_check.c

do_output.o: do_output.c my-trades.h
	$(CC) -c $(MYSQL_FLAGS) do_output.c

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf

install:
	mv ./my-trades /Finance/bin/C/
