/* do_updates.c
 * part of mt2
 * mass-updates of all watchlist stocks that were paper-traded this day
 */

#include	"./mt2.h"

int	do_updates() {
    char	query[1024];
  
// BUYS
  // udpate paper trade buys based on low price
  sprintf(query,"update %s.watchlist w, %s.stockprices b \
    set w.PAPER_BUY_PRICE = w.MEDIAN_BUY, w.STOP_PRICE = w.MEDIAN_BUY - (w.MEDIAN_BUY * 0.05) \
    where w.date = \"%s\" \
    and b.date = \"%s\" \
    and w.SYMBOL = b.symbol \
    and w.PAPER_BUY_PRICE is null \
    and b.day_low is not null \
    and b.day_low > 0 \
    and b.day_low <= w.MEDIAN_BUY",
    DB_INVESTMENTS, DB_BEANCOUNTER, priorDate, qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update watchlist database PAPER_BUY_PRICE"); }

// SELLS
  // update paper trade sells based on high price
  sprintf(query,"update %s.watchlist w, %s.stockprices b \
    set w.PAPER_SELL_PRICE = w.AVG_SELL_NEXT \
    where w.date = \"%s\" \
    and b.date = \"%s\" \
    and w.symbol=b.symbol \
    and w.PAPER_BUY_PRICE is not null \
    and w.PAPER_BUY_PRICE > 0 \
    and w.PAPER_SELL_PRICE is null \
    and w.AVG_SELL_NEXT is not null \
    and w.AVG_SELL_NEXT > 0 \
    and b.day_high is not null \
    and b.day_high >= w.AVG_SELL_NEXT",
    DB_INVESTMENTS, DB_BEANCOUNTER, prevpriorDate, qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update watchlist database PAPER_SELL_PRICE"); }
  
  // if we didn't hit the target price, did we at least hit the original buy price?
  sprintf(query,"update %s.watchlist w, %s.stockprices b \
    set w.PAPER_SELL_PRICE = w.PAPER_BUY_PRICE \
    where w.date = \"%s\" \
    and b.date = \"%s\" \
    and w.symbol=b.symbol \
    and w.PAPER_BUY_PRICE is not null \
    and w.PAPER_BUY_PRICE > 0 \
    and w.PAPER_SELL_PRICE is null \
    and b.day_high is not null \
    and b.day_high >= w.PAPER_BUY_PRICE \
    and b.day_high < w.AVG_SELL_NEXT",
    DB_INVESTMENTS, DB_BEANCOUNTER, prevpriorDate, qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update watchlist database PAPER_SELL_PRICE"); }

  // did we hit the stop-loss price?
  sprintf(query,"update %s.watchlist w, %s.stockprices b \
    set w.PAPER_SELL_PRICE = w.STOP_PRICE \
    where w.date = \"%s\" \
    and b.date = \"%s\" \
    and w.symbol=b.symbol \
    and w.PAPER_BUY_PRICE is not null \
    and w.PAPER_BUY_PRICE > 0 \
    and w.PAPER_SELL_PRICE is null \
    and b.day_high is not null \
    and b.day_high > 0 \
    and b.day_high < w.PAPER_BUY_PRICE \
    and b.day_low is not null \
    and b.day_low > 0 \
    and b.day_low <= w.STOP_PRICE",
    DB_INVESTMENTS, DB_BEANCOUNTER, prevpriorDate, qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update watchlist database PAPER_SELL_PRICE"); }
  
  // had to sell for a loss but not stopped out
  sprintf(query,"update %s.watchlist w, %s.stockprices b \
    set w.PAPER_SELL_PRICE = b.day_high \
    where w.date = \"%s\" \
    and b.date = \"%s\" \
    and w.symbol=b.symbol \
    and w.PAPER_BUY_PRICE is not null \
    and w.PAPER_BUY_PRICE > 0 \
    and w.PAPER_SELL_PRICE is null \
    and b.day_high is not null \
    and b.day_high > 0 \
    and b.day_high < w.PAPER_BUY_PRICE ",
    DB_INVESTMENTS, DB_BEANCOUNTER, prevpriorDate, qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update watchlist database PAPER_SELL_PRICE"); }
  
  return EXIT_SUCCESS;
}
