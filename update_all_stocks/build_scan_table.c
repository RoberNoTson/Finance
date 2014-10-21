/* build_scan_table.c
 * part of update_all_stocks
 */

#include "update_all_stocks.h"

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
  // get the potential buys to watch
  sprintf(query,"select distinct SYMBOL from beancounter.stockinfo where active");
  if (mysql_query(mysql,query)) { puts("04 Failed to query watchlist database in build_scan_table"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("04 Failed to store results for watchlist database in build_scan_table"); return(EXIT_FAILURE); }
  if ((num_rows=mysql_num_rows(result)) == 0) { puts("04 No buy rows found in build_scan_table"); return(EXIT_FAILURE); }
  // add BUYS to table
  for (x=0;x<num_rows;x++) {
    num_stocks++;
    if ((stock_table=realloc(stock_table,sizeof(struct stocks) * num_stocks)) == NULL) {
      puts("01 realloc failed in build_scan_table");
      return(EXIT_FAILURE);
    }
    Stocks = (struct stocks *) &stock_table[(num_stocks-1)*sizeof(struct stocks)];
    row=mysql_fetch_row(result);
    strcpy(Stocks->SYMBOL,row[0]);
  }	// end For loop

   // finished with the database
  mysql_free_result(result);
  #include	"/Finance/bin/C/src/Includes/mysql-disconn.inc"
  return num_stocks;
}
