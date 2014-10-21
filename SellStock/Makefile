# Makefile for sell_stock
#
# 
CC=/usr/bin/gcc
CFLAGS=-Wall -O2 -ffast-math
MYSQL_FLAGS=`mysql_config --include --libs`
LDFLAGS=$(MYSQL_FLAGS)

sell_stock: sell_stock.o create_menu.o get_input.o update_Investments_db.o update_beancounter_db.o update_watchlist.o
	$(CC) $(CFLAGS) sell_stock.o create_menu.o get_input.o update_Investments_db.o update_beancounter_db.o update_watchlist.o $(LDFLAGS) -o sell_stock

sell_stock.o: sell_stock.c sell_stock.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) sell_stock.c

create_menu.o: create_menu.c sell_stock.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) create_menu.c

get_input.o: get_input.c sell_stock.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) get_input.c

update_Investments_db.o: update_Investments_db.c sell_stock.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) update_Investments_db.c

update_beancounter_db.o: update_beancounter_db.c sell_stock.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) update_beancounter_db.c

update_watchlist.o: update_watchlist.c sell_stock.h
	$(CC) -c $(CFLAGS) $(MYSQL_FLAGS) update_watchlist.c

install:
	mv ./sell_stock /Finance/bin/C/

clean:
	rm -f *.o
	rm -f *~
	rm -f asdf
