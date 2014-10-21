/* update_Investments_db.c
 * part of sell_stock
 */
#include	"sell_stock.h"

int	update_Investments_db(void) {
  char	query[1024];
  
  // basic case, sell all shares from single Lot
  if (Shares == buy_shares && buy_shares <= holdings) {
    sprintf(query,"update Investments.activity \
      set sell_date=\"%s\", sell_price=%.4f, sell_shares=%d, sell_fees=%.2f where ID = %d \
      and sell_date is null and sell_price = 0",
      sDate,Price,Shares,Fees+Comm,ID);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if(mysql_query(mysql,query)) { puts("Failed to update database"); return(EXIT_FAILURE); }
    
  }	// initial partial sale of a single Lot
  else if (Shares < buy_shares && Shares < holdings) {
    sprintf(query,"update Investments.activity \
      set sell_date=\"%s\", sell_price=%.4f, sell_shares=%d, sell_fees=%.2f where Lot_id = %d \
      and sell_date is null and sell_price = 0",
      sDate,Price,Shares,Fees+Comm,ID);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if(mysql_query(mysql,query)) { puts("Failed to update database"); return(EXIT_FAILURE); }
    sprintf(query,"insert into Investments.activity (symbol,buy_date,buy_price,Lot_id) \
    values (\"%s\",\"%s\",%.4f,%d)", Sym,buy_date,buy_price,ID);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if(mysql_query(mysql,query)) { puts("Failed to update database"); return(EXIT_FAILURE); }
    
  }	// secondary sale of partial Lot
  else if (Shares < buy_shares && Shares == holdings) {
    sprintf(query,"update Investments.activity \
    set sell_date=\"%s\", sell_price=%.4f, sell_shares=%d, sell_fees=%.2f \
    where symbol = \"%s\" and buy_date = \"%s\" and buy_price = %.4f and buy_shares = 0 \
    and sell_date is null and sell_price = 0",
    sDate,Price,Shares,Fees+Comm,Sym,buy_date,buy_price);
    if (DEBUG) printf("03 %s\n",query);
    if (!DEBUG) if(mysql_query(mysql,query)) { puts("Failed to update database"); return(EXIT_FAILURE); }
  }
  updated_holdings = holdings - Shares;
  if (DEBUG) printf("updated_holdings = %d\n",updated_holdings);

  return(EXIT_SUCCESS);
}
