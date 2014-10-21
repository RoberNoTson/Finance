/* scan_db.c
 * main production part of watchlist_monitor.c
 * 1) build a memory table of all selected potential buy/sell stocks
 * 2) loop continually through the memory table while the market is open, updating buy/sell data
 */
#include "watchlist_monitor.h"
  
int	scan_db(void) {
  int	num_stocks;
  int	num_holdings=0;
  pthread_t	watchlist_pid=0;
  pthread_t	holdings_pid=0;
  
  // allocate memory and initialize things
  if ((chunk.memory = calloc(1,1)) == NULL) {
    puts("chunk calloc failed in scan_db, aborting process");
    return(EXIT_FAILURE);
  }    
  if ((holdings_table=calloc(1,1)) == NULL) { 
    puts("Calloc error for build_holdings_table in scan_db"); 
    return(EXIT_FAILURE); 
  }
  if ((stock_table=calloc(1,1)) == NULL) { 
    puts("Calloc error for build_scan_table in scan_db"); 
    return(EXIT_FAILURE); 
  }
  
  // build memory tables of stocks to scan
  if ((num_stocks = build_scan_table()) == EXIT_FAILURE) { puts("Error building scan table"); exit(EXIT_FAILURE); }
  if (debug) printf("num_stocks to scan = %d\n",num_stocks);
  
  // big infinite loop to process through the stock table
  if (debug) printf("Beginning scan_db\n");
  while(1) {
    if (time_check()) if (!debug) { raise(SIGINT); return(EXIT_FAILURE); }     // is the market open?
    pthread_create(&holdings_pid,NULL, (void *)scan_holdings, NULL);
    pthread_join(holdings_pid,NULL);
      if (debug) puts("completed holdings scan");
    pthread_create(&watchlist_pid, NULL, (void *)scan_watchlist, (int *)num_stocks);
    pthread_join(watchlist_pid,NULL);
      if (debug) puts("completed watchlist child thread");
      if (debug) printf("Pausing for %d seconds\n",interval);
    sleep(interval);
  } // end While infinite loop
  
  free(stock_table);
  free(holdings_table);
  free(chunk.memory);
  return(EXIT_SUCCESS);
}