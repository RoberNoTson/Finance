/* create_menu.c
 * part of sell_stock
 */
#include	"sell_stock.h"
int	Lot_size=0;

int	create_menu(void) {
  int	num_rows;
  int	menu_sel;
  int	x;
  char	query[1024];

  if (DEBUG) puts("Creating menu");
  sprintf(query,"select date from Investments.watchlist where date < \"%s\" order by date desc limit 1",qDate);
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) { puts("04 Failed to query database for dates"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("05 Failed to store dates"); return(EXIT_FAILURE); }
  row = mysql_fetch_row(result);
  strcpy(prevDate,row[0]);
  mysql_free_result(result);
  if (!strlen(Sym)) {
    // select all holdings for menu
    sprintf(query,"select p.symbol, a.buy_date, a.buy_price, p.current_shares, a.buy_shares, a.ID, \
    (select sum(buy_shares)-sum(sell_shares) from Investments.activity where symbol=p.symbol \
    and Lot_id=a.ID group by Lot_id) as lot_size \
    from Investments.portfolio p, Investments.activity a \
    where p.current_shares > 0 \
    and a.symbol=p.symbol \
    and a.buy_shares > a.sell_shares \
    order by symbol, buy_date, current_shares");
    if (DEBUG) printf("%s\n",query);
    if (mysql_query(mysql,query)) { puts("05 Failed to query database"); return(EXIT_FAILURE); }
  } else {
    // Sym passed on CLI, search only for that
    sprintf(query,"select p.symbol, a.buy_date, a.buy_price, p.current_shares, a.buy_shares, a,ID, \
    (select sum(buy_shares)-sum(sell_shares) from Investments.activity where symbol=p.symbol \
    and Lot_id=a.ID group by Lot_id) as lot_size \
    from Investments.portfolio p, Investments.activity a \
    where p.current_shares > 0 \
    and a.symbol = \"%s\" \
    and a.symbol=p.symbol \
    and a.buy_shares > a.sell_shares \
    order by symbol, buy_date, p.current_shares",Sym);
    if (DEBUG) printf("%s\n",query);
    if (mysql_query(mysql,query)) { puts("06 Failed to query database"); return(EXIT_FAILURE); }
  }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) { puts("store_results failed"); return(EXIT_FAILURE); }
  num_rows=mysql_num_rows(result);
  if (!num_rows) {
    puts("Nothing found to sell!");
    return(EXIT_FAILURE);
  }
  if (num_rows == 1) {
    puts("Selected\tBuy Date\tPrice\t\tShares");
    row=mysql_fetch_row(result);
    printf("\t%s\t%s\t%8s\t%s [of %s]\n",row[0],row[1],row[2],row[6],row[3]);
  } else {
    puts("Available shares\nSelect\tSymbol\tDate\t\tPrice\t\tShares");
    // create selection menu of stocks, get input
    for (x=1;x<=num_rows;x++) {
      row=mysql_fetch_row(result);
      if(row==NULL) break;
      if (atoi(row[6])==0) continue;
      printf("%d\t%s\t%s\t%8s\t%s [of %s]\n",x,row[0],row[1],row[2],row[6],row[3]);
    }
    printf("Choice: ");
    fgets(query,sizeof(query),stdin);
    if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
    if (!strlen(query)) { puts("No valid selection found"); return(EXIT_FAILURE); }
    errno=0;
    if ((x=sscanf(query,"%d",&menu_sel)) == 0) { puts("Invalid selection"); return(EXIT_FAILURE); }
    else if (errno)  { puts("Read error"); return(EXIT_FAILURE); }
    else if (x == EOF) { puts("No valid selection made"); return(EXIT_FAILURE); }
    else if (menu_sel > num_rows) { puts("Selection out of range"); return(EXIT_FAILURE); }
    mysql_data_seek(result,menu_sel-1);
    row=mysql_fetch_row(result);
    if (DEBUG) printf("Selected: %d\t%s\t%s\t%s\t%s\t%s\t%s\n",menu_sel,row[0],row[1],row[2],row[3],row[4],row[5]); 
  }	// end If Else
  strcpy(Sym,row[0]);	// portfolio.symbol
  strcpy(buy_date,row[1]);	// activity.buy_date
  buy_price=strtof(row[2],NULL);  // activity.buy_price
  holdings=atoi(row[3]);  // portfolio.current_shares
  updated_holdings = holdings;
  buy_shares=atoi(row[4]);  // activity.buy_shares
  ID=atoi(row[5]);	// unique sequence ID
  Lot_size=atoi(row[6]);	// sum(buy_shares)-sum(sell_shares)
  if (DEBUG) printf("Sym %s buy_date %s buy_price %.4f holdings %d buy_shares %d ID %d\n",Sym,buy_date,buy_price,holdings,buy_shares,ID);
//  "holdings" = `select sum(buy_shares)-sum(sell_shares) from activity where symbol = "SYM" and buy_date = "2012-07-20"`
  mysql_free_result(result);
  sprintf(query,"select sum(buy_shares)-sum(sell_shares) from Investments.activity where symbol = \"%s\" and buy_date = \"%s\"",Sym,buy_date);
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) { puts("07 Failed to query database"); return(EXIT_FAILURE); }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) { puts("store_results failed"); return(EXIT_FAILURE); }
  row=mysql_fetch_row(result);
  holdings = atoi(row[0]);
  if (DEBUG) printf("holdings: %d\n",holdings);
  printf("Selling ticker:\t%s\n",Sym);
  // finished with the database
  mysql_free_result(result);

  return(EXIT_SUCCESS);
}
