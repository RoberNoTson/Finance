/* scan_watchlist_parse.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"

int	scan_watchlist_parse(int stock_offset) {
  float	cur_price=0.0;
  int	x,num_rows,rc=EXIT_SUCCESS;
  unsigned long	*lengths;
  char	msg[1024];
  char	query[1024];
  struct stocks *Stocks;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  
  x = sizeof(struct stocks) * stock_offset;
  Stocks = (struct stocks *) &stock_table[x];
  if (debug) printf("Starting scan_watchlist processing for %s at offset %d\n",Stocks->SYMBOL,stock_offset);
  memset(msg,0,sizeof(msg));
  
  // get real-time price for the selected stock symbol
  cur_price = get_RTquote(Stocks->SYMBOL);
  if (cur_price < 1) { if (debug) printf("%s cur_price < 1, giving up\n",Stocks->SYMBOL); exit(EXIT_FAILURE); }
  if (debug) printf("%s Watchlist cur_price = %.2f\n",Stocks->SYMBOL,cur_price);
   
  // get current data from db
  #include	"/Finance/bin/C/src/Includes/beancounter-conn.inc"
  sprintf(query,"select SYMBOL,PAPER_BUY_PRICE,PAPER_SELL_PRICE,STOP_PRICE,MEDIAN_BUY,AVG_SELL, \
    MEDIAN_SELL,AVG_SELL_NEXT,MEDIAN_SELL_NEXT \
      from Investments.watchlist where SYMBOL = \"%s\" and date = \"%s\"",Stocks->SYMBOL,Stocks->date);
  if (debug) printf("%s\n",query);
  if (mysql_query(mysql,query)) { puts("04 Failed to query watchlist database in scan_watchlist_parse"); rc=EXIT_FAILURE; goto do_exit; }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("04 Failed to store results for watchlist database in scan_watchlist_parse"); rc=EXIT_FAILURE; goto do_exit; }
  if ((num_rows=mysql_num_rows(result)) == 0) { puts("04 No rows found in scan_watchlist_parse"); rc=EXIT_FAILURE; goto do_exit; }
  if (num_rows > 1) { puts("Too many rows returned in scan_watchlist_parse"); rc=EXIT_FAILURE; goto do_exit; }
  row=mysql_fetch_row(result);
  lengths=mysql_fetch_lengths(result);
  if (lengths[1]) Stocks->PAPER_BUY_PRICE = strtof(row[1],NULL);
      else Stocks->PAPER_BUY_PRICE = 0;
  if (lengths[2]) Stocks->PAPER_SELL_PRICE = strtof(row[2],NULL);
      else Stocks->PAPER_SELL_PRICE = 0;
  if (lengths[3]) Stocks->STOP_PRICE = strtof(row[3],NULL);
      else Stocks->STOP_PRICE = 0;
  if (lengths[4]) Stocks->MEDIAN_BUY = strtof(row[4],NULL);
      else {
	if (debug) printf("Invalid MEDIAN_BUY for %s\n",Stocks->SYMBOL); 
	rc=EXIT_FAILURE; goto do_exit;
      }
  if (lengths[5]) Stocks->AVG_SELL = strtof(row[5],NULL);
      else {
	if (debug) printf("Invalid AVG_SELL for %s\n",Stocks->SYMBOL); 
	rc=EXIT_FAILURE; goto do_exit;
      }
  if (lengths[6]) Stocks->MEDIAN_SELL = strtof(row[6],NULL);
      else {
	if (debug) printf("Invalid MEDIAN_SELL for %s\n",Stocks->SYMBOL); 
	rc=EXIT_FAILURE; goto do_exit;
      }
  if (lengths[7]) Stocks->AVG_SELL_NEXT = strtof(row[7],NULL);
    else Stocks->AVG_SELL_NEXT = 0;
  if (lengths[8]) Stocks->MEDIAN_SELL_NEXT = strtof(row[8],NULL);
    else Stocks->MEDIAN_SELL_NEXT = 0;
  memset(query,0,sizeof(query));
  
    // basic condition for buying a stock
      if (Stocks->PAPER_BUY_PRICE == 0
	&& strcmp(Stocks->date,buyDate) == 0
	&& cur_price <= Stocks->MEDIAN_BUY) 
      {
	// mark it as bought
	Stocks->PAPER_BUY_PRICE = Stocks->MEDIAN_BUY;
	Stocks->STOP_PRICE = Stocks->PAPER_BUY_PRICE - (Stocks->PAPER_BUY_PRICE * STOPLOSS_PERCENT);
	sprintf(msg,"Buy %s at %.2f",Stocks->SYMBOL,cur_price);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_BUY_PRICE = %.4f, STOP_PRICE = %.4f \
	  where SYMBOL = \"%s\" and date = \"%s\"",
	  Stocks->PAPER_BUY_PRICE,Stocks->STOP_PRICE,Stocks->SYMBOL,Stocks->date);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }

  // maximum same-day sale
      if (cur_price >= Stocks->MEDIAN_SELL
	&& cur_price > Stocks->PAPER_BUY_PRICE
	&& strcmp(Stocks->date,buyDate) == 0
	&& Stocks->PAPER_BUY_PRICE > 0
	&& Stocks->PAPER_SELL_PRICE < Stocks->MEDIAN_SELL) 
      {
	// mark it as sold
	Stocks->PAPER_SELL_PRICE = Stocks->MEDIAN_SELL;
	sprintf(msg,"Sell %s at %.2f  same-day max gain",Stocks->SYMBOL,Stocks->MEDIAN_SELL);
	if (debug) printf("**** %s\n",msg);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_SELL_PRICE = %.4f \
	where SYMBOL = \"%s\" and date = \"%s\" and PAPER_BUY_PRICE = %.4f",
	 Stocks->MEDIAN_SELL,Stocks->SYMBOL,Stocks->date,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }
      
    // average same-day sale
      if (cur_price >= Stocks->AVG_SELL
	&& cur_price < Stocks->MEDIAN_SELL 
	&& cur_price > Stocks->PAPER_BUY_PRICE
	&& strcmp(Stocks->date,buyDate) == 0
	&& Stocks->PAPER_BUY_PRICE > 0
	&& Stocks->PAPER_SELL_PRICE < Stocks->AVG_SELL)
      {
	// mark it as sold
	Stocks->PAPER_SELL_PRICE = Stocks->AVG_SELL;
	sprintf(msg,"Sell %s at %.2f  same-day avg gain",Stocks->SYMBOL,Stocks->AVG_SELL);
	if (debug) printf("**** %s\n",msg);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_SELL_PRICE = %.4f \
	where SYMBOL = \"%s\" and date = \"%s\" and PAPER_BUY_PRICE = %.4f",
	 Stocks->AVG_SELL,Stocks->SYMBOL,Stocks->date,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }

    // Maximum sell the next day
      if (Stocks->PAPER_BUY_PRICE > 0
	&& strcmp(Stocks->date,sellDate) == 0
	&& cur_price >= Stocks->MEDIAN_SELL_NEXT
	&& cur_price > Stocks->AVG_SELL_NEXT
	&& Stocks->PAPER_BUY_PRICE < Stocks->MEDIAN_SELL_NEXT
	&& Stocks->PAPER_SELL_PRICE < Stocks->MEDIAN_SELL_NEXT) 
      {
	// mark it as sold
	Stocks->PAPER_SELL_PRICE = Stocks->MEDIAN_SELL_NEXT;
	sprintf(msg,"Sell %s at %.2f  max gain",Stocks->SYMBOL,Stocks->MEDIAN_SELL_NEXT);
	if (debug) printf("**** %s\n",msg);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_SELL_PRICE = %.4f \
	where SYMBOL = \"%s\" and date = \"%s\" and PAPER_BUY_PRICE = %.4f",
	 Stocks->MEDIAN_SELL_NEXT,Stocks->SYMBOL,Stocks->date,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }

    // Average sell the next day
      if (Stocks->PAPER_BUY_PRICE > 0
	&& strcmp(Stocks->date,sellDate) == 0
	&& cur_price >= Stocks->AVG_SELL_NEXT
	&& cur_price < Stocks->MEDIAN_SELL_NEXT
	&& Stocks->PAPER_BUY_PRICE < Stocks->AVG_SELL_NEXT
	&& Stocks->PAPER_SELL_PRICE < Stocks->AVG_SELL_NEXT)
      {
	// mark it as sold
	Stocks->PAPER_SELL_PRICE = Stocks->AVG_SELL_NEXT;
	sprintf(msg,"Sell %s at %.2f  avg gain",Stocks->SYMBOL,Stocks->AVG_SELL_NEXT);
	if (debug) printf("**** %s\n",msg);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_SELL_PRICE = %.4f \
	where SYMBOL = \"%s\" and date = \"%s\" and PAPER_BUY_PRICE = %.4f",
	 Stocks->AVG_SELL_NEXT,Stocks->SYMBOL,Stocks->date,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }

    // break-even sell the next day
      if (Stocks->PAPER_BUY_PRICE > 0
	&& strcmp(Stocks->date,sellDate) == 0
	&& cur_price >= Stocks->PAPER_BUY_PRICE
	&& Stocks->PAPER_SELL_PRICE == 0)
      {
	// mark it as sold
	Stocks->PAPER_SELL_PRICE = Stocks->PAPER_BUY_PRICE;
	sprintf(msg,"Sell %s at %.2f  break-even",Stocks->SYMBOL,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("**** %s\n",msg);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_SELL_PRICE = %.4f \
	where SYMBOL = \"%s\" and date = \"%s\" and PAPER_BUY_PRICE = %.4f",
	 Stocks->PAPER_BUY_PRICE,Stocks->SYMBOL,Stocks->date,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }
      
    // stop-loss
      if (Stocks->PAPER_BUY_PRICE > 0
	&& Stocks->PAPER_SELL_PRICE == 0
	&& cur_price <= Stocks->STOP_PRICE) 
      {
	// mark it as sold
	Stocks->PAPER_SELL_PRICE = Stocks->STOP_PRICE;
	sprintf(msg,"Sell %s at %.2f stop-loss",Stocks->SYMBOL,Stocks->STOP_PRICE);
	if (debug) printf("**** %s\n",msg);
	// update database
	sprintf(query,"update Investments.watchlist set PAPER_SELL_PRICE = %.4f \
	where SYMBOL = \"%s\" and date = \"%s\" and PAPER_BUY_PRICE = %.4f",
	 Stocks->STOP_PRICE,Stocks->SYMBOL,Stocks->date,Stocks->PAPER_BUY_PRICE);
	if (debug) printf("%s\n",query);
	if (!debug) mysql_query(mysql,query);
      }
  do_exit:
  // finished with the database
  mysql_free_result(result);
//  mysql_thread_end();
  #include	"/Finance/bin/C/src/Includes/mysql-disconn.inc"
  // process the output message
  if (debug) printf("completed scan_watchlist processing for %s\n",Stocks->SYMBOL);
  if (rc == EXIT_SUCCESS) do_output(msg);
  exit(rc);
}
