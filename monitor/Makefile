# Makefile for
# watchlist_monitor 
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O3 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs_r`
CURL_FLAGS=`curl-config --libs`
LDFLAGS=$(MYSQL_FLAGS) $(CURL_FLAGS)


watchlist_monitor: watchlist_monitor.o usage.o ParseData.o ParseConfig.o freeallmem.o cleanup.o sighandlers.o time_check.o scan_db.o build_scan_table.o get_passwd.o get_RTquote.o scan_watchlist.o scan_watchlist_parse.o scan_holdings.o scan_holdings_parse.o do_output.o build_holdings_table.o mon_cnf_file.o
	$(CC) $(CFLAGS) watchlist_monitor.o usage.o ParseData.o ParseConfig.o freeallmem.o cleanup.o sighandlers.o time_check.o scan_db.o build_scan_table.o get_passwd.o get_RTquote.o scan_watchlist.o scan_watchlist_parse.o scan_holdings.o scan_holdings_parse.o do_output.o build_holdings_table.o mon_cnf_file.o $(LDFLAGS) -o watchlist_monitor

watchlist_monitor.o: watchlist_monitor.c watchlist_monitor.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) watchlist_monitor.c

usage.o: usage.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) usage.c

ParseData.o: ParseData.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) ParseData.c

ParseConfig.o: ParseConfig.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) ParseConfig.c

freeallmem.o: freeallmem.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) freeallmem.c

cleanup.o: cleanup.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) cleanup.c

sighandlers.o: sighandlers.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) sighandlers.c

time_check.o: time_check.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) time_check.c

scan_db.o: scan_db.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) scan_db.c

scan_watchlist.o: scan_watchlist.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) scan_watchlist.c

scan_watchlist_parse.o: scan_watchlist_parse.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) scan_watchlist_parse.c

scan_holdings_parse.o: scan_holdings_parse.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) scan_holdings_parse.c

scan_holdings.o: scan_holdings.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) scan_holdings.c

build_scan_table.o: build_scan_table.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) build_scan_table.c

build_holdings_table.o: build_holdings_table.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) build_holdings_table.c

get_passwd.o: get_passwd.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) get_passwd.c

get_RTquote.o: get_RTquote.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) get_RTquote.c

do_output.o: do_output.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) do_output.c

mon_cnf_file.o: mon_cnf_file.c watchlist_monitor.h
	$(CC) -c $(MYSQL_FLAGS) mon_cnf_file.c

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf

install:
	mv /Finance/bin/C/src/monitor/watchlist_monitor /Finance/bin/C/watchlist_monitor_MT
