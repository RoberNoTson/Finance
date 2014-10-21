/* scan_holdings_parse.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"

int	scan_holdings_parse(int stock_offset) {
  float	cur_price=0.0;
  int	x,num_rows,rc=EXIT_SUCCESS;
  unsigned long	*lengths;
  char	msg[1024];
  char	query[1024];
  char	Today[16];
  time_t t;
  struct tm *TM=0;
  struct stocks *Stocks;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  
  
  x = sizeof(struct stocks) * stock_offset;
  Stocks = (struct stocks *) &holdings_table[x];
  if (debug) printf("Starting scan_holdings_parse processing for %s at offset %d\n",Stocks->SYMBOL,stock_offset);
  // get real-time price for the selected stock symbol
  if ((cur_price = get_RTquote(Stocks->SYMBOL)) == 0) exit(EXIT_FAILURE);
  if (cur_price < 1.0) { if (debug) printf("%s cur_price < 1.00, giving up\n",Stocks->SYMBOL); exit(EXIT_FAILURE); }
  if (debug) printf("%s Holding cur_price = %.2f\n",Stocks->SYMBOL,cur_price);
   
  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    return(EXIT_FAILURE);
  }
  if (strftime(Today, sizeof(Today), "%F", TM) == 0) {
    puts("In scan_holdings -  strftime returned 0");
    return(EXIT_FAILURE);
  }

  // parse the data
  memset(msg,0,sizeof(msg));
  if (strncmp(Today,Stocks->date,10)==0 
    && cur_price <= (Stocks->PAPER_BUY_PRICE - (Stocks->PAPER_BUY_PRICE*0.02))) {
    sprintf(msg,"Sell %s purchased at %.2f same-day 2%% StopLoss",Stocks->SYMBOL,Stocks->PAPER_BUY_PRICE); 
  }
  if (cur_price <= (Stocks->PAPER_BUY_PRICE - (Stocks->PAPER_BUY_PRICE*0.07))) { 
    sprintf(msg,"Sell %s purchased at %.2f 7%% StopLoss",Stocks->SYMBOL,Stocks->PAPER_BUY_PRICE); 
  }
  if (cur_price >= (Stocks->PAPER_BUY_PRICE + (Stocks->PAPER_BUY_PRICE*0.05))) { 
    sprintf(msg,"Sell %s purchased at %.2f 5%% gain",Stocks->SYMBOL,Stocks->PAPER_BUY_PRICE); 
  }

  // process the output message
  if (debug) printf("completed scan_holdings processing for %s\n",Stocks->SYMBOL);
  do_output(msg);
  return(EXIT_SUCCESS);
}
