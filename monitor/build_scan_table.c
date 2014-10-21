/* build_scan_table.c
 * part of watchlist_monitor
 */

#include "watchlist_monitor.h"

int	build_scan_table(void) {
  int	num_stocks=0;
  int	x,num_rows;
  unsigned long	*lengths;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

  if (debug) puts("Building scan table");
  // build table of stocks to check
  #include	"/Finance/bin/C/src/Includes/beancounter-conn.inc"
  // get correct dates
  sprintf(query,"select distinct(date) from Investments.watchlist where date < NOW() order by date desc limit 2");
  memset(db_pass,0,sizeof(db_pass));
  if (mysql_query(mysql,query)) { puts("ERROR - Failed to query watchlist database for sell"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("ERROR - Failed to store results for watchlist database in build_scan_table"); return(EXIT_FAILURE); }
  row = mysql_fetch_row(result);
  strcpy(buyDate,row[0]);
  row = mysql_fetch_row(result);
  strcpy(sellDate,row[0]);
  mysql_free_result(result);
  if(debug) printf("Buy date %s\t Sell date %s\n",buyDate,sellDate);
  
  // get the potential buys to watch
  sprintf(query,"select SYMBOL,date,MEDIAN_BUY,AVG_SELL,MEDIAN_SELL,PAPER_BUY_PRICE,PAPER_SELL_PRICE,STOP_PRICE,AVG_SELL_NEXT,MEDIAN_SELL_NEXT \
  from Investments.watchlist where date = \"%s\"",buyDate);
  if (mysql_query(mysql,query)) { printf("ERROR - Failed to query watchlist database in %s\n",__FILE__); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { printf("ERROR - Failed to store results for watchlist database in %s\n"__FILE__); return(EXIT_FAILURE); }
  if ((num_rows=mysql_num_rows(result)) == 0) { printf("WARNING - No buy rows found in %s\n",__FILE__); }
  // add BUYS to table
  for (x=0;x<num_rows;x++) {
    num_stocks++;
    if ((stock_table=realloc(stock_table,sizeof(struct stocks) * num_stocks)) == NULL) {
      printf("ERROR - realloc failed in %s\n",__FILE__);
      return(EXIT_FAILURE);
    }
    Stocks = (struct stocks *) &stock_table[(num_stocks-1)*sizeof(struct stocks)];
    row=mysql_fetch_row(result);
    lengths = mysql_fetch_lengths(result);
    strcpy(Stocks->SYMBOL,row[0]);
    strcpy(Stocks->date,row[1]);
    if (lengths[2]) Stocks->MEDIAN_BUY = strtof(row[2],NULL);
    else Stocks->MEDIAN_BUY = 0;
    if (lengths[3]) Stocks->AVG_SELL = strtof(row[3],NULL);
    else Stocks->AVG_SELL = 0;
    if (lengths[4]) Stocks->MEDIAN_SELL = strtof(row[4],NULL);
    else Stocks->MEDIAN_SELL = 0;
    if (lengths[5]) Stocks->PAPER_BUY_PRICE = strtof(row[5],NULL);
    else Stocks->PAPER_BUY_PRICE = 0;
    if (lengths[6]) Stocks->PAPER_SELL_PRICE = strtof(row[6],NULL);
    else Stocks->PAPER_SELL_PRICE = 0;
    if (lengths[7]) Stocks->STOP_PRICE = strtof(row[7],NULL);
    else Stocks->STOP_PRICE = 0;
    if (lengths[8]) Stocks->AVG_SELL_NEXT = strtof(row[8],NULL);
    else Stocks->AVG_SELL_NEXT = 0;
    if (lengths[9]) Stocks->MEDIAN_SELL_NEXT = strtof(row[9],NULL);
    else Stocks->MEDIAN_SELL_NEXT = 0;
  }	// end For loop

  // get the potential sells to watch when they match
//  sprintf(query,"select SYMBOL,date,PAPER_BUY_PRICE,AVG_SELL_NEXT,MEDIAN_SELL_NEXT,PAPER_SELL_PRICE,STOP_PRICE 
  sprintf(query,"select SYMBOL,date,MEDIAN_BUY,AVG_SELL,MEDIAN_SELL,PAPER_BUY_PRICE,PAPER_SELL_PRICE,STOP_PRICE,AVG_SELL_NEXT,MEDIAN_SELL_NEXT \
  from Investments.watchlist  where date = \"%s\" and PAPER_BUY_PRICE is not null",sellDate);
  if (debug) printf("watching %s\n",query);
  if (mysql_query(mysql,query)) { printf("ERROR 05 Failed to query watchlist database in %s\n",__FILE__); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { printf("ERROR 06 Failed to store results for watchlist database in %s\n",__FILE__); return(EXIT_FAILURE); }
  // NOTE: if there were no buys the previous day then 0 rows would be correctly returned!
  if ((num_rows=mysql_num_rows(result)) == 0) { printf("WARNING No sell rows found in %s\n",__FILE__); }
  // add SELLS to table
  for (x=0;x<num_rows;x++) {
    num_stocks++;
    if ((stock_table=realloc(stock_table,sizeof(struct stocks) * num_stocks)) == NULL) {
      printf("ERROR 02 realloc failed in %s\n",__FILE__);
      return(EXIT_FAILURE);
    }
    Stocks = (struct stocks *) &stock_table[(num_stocks-1)*sizeof(struct stocks)];
    row=mysql_fetch_row(result);
    lengths = mysql_fetch_lengths(result);
    strcpy(Stocks->SYMBOL,row[0]);
    strcpy(Stocks->date,row[1]);
    if (lengths[2]) Stocks->MEDIAN_BUY = strtof(row[2],NULL);
      else {
	printf("Invalid MEDIAN_BUY for %s\n",Stocks->SYMBOL); 
	continue; 
      }
    if (lengths[3]) Stocks->AVG_SELL = strtof(row[3],NULL);
      else {
	printf("Invalid AVG_SELL for %s\n",Stocks->SYMBOL); 
	continue; 
      }
    if (lengths[4]) Stocks->MEDIAN_SELL = strtof(row[4],NULL);
      else {
	printf("Invalid MEDIAN_SELL for %s\n",Stocks->SYMBOL); 
	continue; 
      }
    if (lengths[5]) Stocks->PAPER_BUY_PRICE = strtof(row[5],NULL);
      else {
	printf("Invalid PAPER_BUY_PRICE for %s\n",Stocks->SYMBOL); 
	continue; 
      }
    if (lengths[6]) Stocks->PAPER_SELL_PRICE = strtof(row[6],NULL);
    else Stocks->PAPER_SELL_PRICE = 0;
    if (lengths[7]) Stocks->STOP_PRICE = strtof(row[7],NULL);
      else {
	printf("Invalid STOP_PRICE for %s\n",Stocks->SYMBOL); 
	continue; 
      }
    if (lengths[8]) Stocks->AVG_SELL_NEXT = strtof(row[8],NULL);
      else {
	printf("Invalid AVG_SELL_NEXT for %s\n",Stocks->SYMBOL); 
	continue; 
      }
    if (lengths[9]) Stocks->MEDIAN_SELL_NEXT = strtof(row[9],NULL);
      else {
	printf("Invalid MEDIAN_SELL_NEXT for %s\n",Stocks->SYMBOL); 
	continue; 
      }
  }	// end For loop
  if (!num_stocks) puts("No potential sale stocks found to track in watchlist");
  // table of stocks is now built
  if (debug) puts("stock_table now populated");
   // finished with the database
  mysql_free_result(result);
  #include	"/Finance/bin/C/src/Includes/mysql-disconn.inc"
  return num_stocks;
}
