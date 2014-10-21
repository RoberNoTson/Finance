/* update_beancounter_db.c
 * part of sell_stock
 */
#include	"sell_stock.h"

int	update_beancounter_db(void) {
  char	query[1024];

  // Update beancounter portfolio and cash. 
  // basic case, sell all shares of a stock
  if (DEBUG) printf("In update_beancounter_db updated_holdings %d\tholdings %d\tShares %d\tbuy_shares %d\n",updated_holdings,holdings,Shares,buy_shares);
  if (updated_holdings == 0 || Shares == buy_shares) {
    sprintf(query,"delete from beancounter.portfolio where symbol = \"%s\" \
      and date = \"%s\" and shares = %d and cost = %.2f limit 1",
      Sym,buy_date,Shares,buy_price);
  } // partial sale of a single buy
  else if (Shares < buy_shares && Shares <= holdings) {
    sprintf(query,"update beancounter.portfolio set shares = shares - %d where symbol = \"%s\" \
      and date = \"%s\" and cost = %.2f and shares >= %d limit 1",
      Shares,Sym,buy_date,buy_price,Shares);
  }
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) { puts("01 Failed to update beancounter database"); return(EXIT_FAILURE); }
  // update cash table
  sprintf(query,"insert into beancounter.cash (value,type,cost,date,name) values(%.2f,\"Sell\",%.2f,\"%s\",\"ML-CMA\")",
	  (Shares*Price),Fees+Comm,sDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) { puts("02 Failed to update beancounter database"); return(EXIT_FAILURE); }

  return(EXIT_SUCCESS);
}
