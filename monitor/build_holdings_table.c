/* build_holdings_table.c
 * part of watchlist_monitor
 */

#include "watchlist_monitor.h"

int	build_holdings_table(void) {
  static int	num_holdings=0;
  int	x,num_rows;
  unsigned long	*lengths;
  char	query[1024];
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

    if (debug) puts("Building holdings table");
  // build table of stocks to check
  #include	"/Finance/bin/C/src/Includes/beancounter-conn.inc"
  // get the potential buys to watch
  if (mysql_query(mysql,"select symbol,date,cost from beancounter.portfolio order by symbol,date")) {
      puts("scan_holdings failed to query portfolio"); 
      return(EXIT_FAILURE); 
  }
    if (debug) puts("Holdings query completed");
  if ((result=mysql_store_result(mysql)) == NULL) {
    puts("04 Failed to store results for holdings in scan_holdings");
    return(EXIT_FAILURE);
  }
  num_rows=mysql_num_rows(result);
  if (!num_rows) { 
    puts("Nothing found to track in holdings"); 
    exit(EXIT_FAILURE);
  }
    if (debug) printf("Getting prices for %d holdings\n",num_rows);
  num_holdings=0;
  for (x=0;x<num_rows;x++) {
    num_holdings++;
    if ((holdings_table=realloc(holdings_table,sizeof(struct stocks) * num_holdings)) == NULL) {
      puts("01 realloc failed in build_holdings_table");
      return(EXIT_FAILURE);
    }
    Stocks = (struct stocks *) &holdings_table[(num_holdings-1)*sizeof(struct stocks)];
    row=mysql_fetch_row(result);
    lengths = mysql_fetch_lengths(result);
    strcpy(Stocks->SYMBOL,row[0]);
    strcpy(Stocks->date,row[1]);
    if (lengths[2]) Stocks->PAPER_BUY_PRICE = strtof(row[2],NULL);
    else {
      Stocks->PAPER_BUY_PRICE = 0;
      printf("Invalid PAPER_BUY_PRICE for %s\n",Stocks->SYMBOL); 
      continue;
    }
  }	// end FOR
    if (debug) puts("holdings_table now populated");
  // finished with the database
  mysql_free_result(result);
  #include	"/Finance/bin/C/src/Includes/mysql-disconn.inc"
  return num_holdings;
}
