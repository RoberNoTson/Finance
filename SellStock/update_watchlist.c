/* update_watchlist.c
 * part of sell_stock
 */

#include	"sell_stock.h"

int	update_watchlist(void) {
  char	query[1024];

  // update watchlist where it matches
  sprintf(query,"select date from Investments.watchlist where date < \"%s\" order by date desc limit 1",buy_date);
  if (mysql_query(mysql,query)) { puts("04 Failed to query watchlist database for buy_date"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("05 Error storing result for watchlist date"); return(EXIT_FAILURE); }
  if (mysql_num_rows(result)) {
    row = mysql_fetch_row(result);
    strcpy(prevDate,row[0]);
    mysql_free_result(result);
    sprintf(query,"update Investments.watchlist set REAL_SELL_PRICE = %.4f where SYMBOL = \"%s\" \
      and date = \"%s\" and REAL_BUY_PRICE is not null and REAL_SELL_PRICE is null",
      Price,Sym,prevDate);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if (mysql_query(mysql,query)) { puts("06 Failed to update watchlist database for Sell"); return(EXIT_FAILURE); }
  }
  
  return(EXIT_SUCCESS);
}
