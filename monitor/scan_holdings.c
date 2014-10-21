/* scan_holdings.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"

int	scan_holdings(void) {
  int	x,num_holdings,y,rem;
  pthread_t	holdings_parse[MAXPARA];
  
    if (debug) puts("Getting current prices for holdings");
  // build/refresh the list of current holdings. It can change during the trading day.
  if ((num_holdings = build_holdings_table()) == EXIT_FAILURE) { 
    puts("Error building holdings table"); 
    exit(EXIT_FAILURE); 
  }
  // exit early if no holdings are found
  if (!num_holdings) exit(EXIT_FAILURE);
  rem = num_holdings % MAXPARA;
  
  for (x=0;x<(num_holdings-rem);x+=MAXPARA) {
    for (y=0;y<MAXPARA;y++) {
      holdings_parse[y] = fork();
      if (holdings_parse[y] == -1) {
        perror("fork attempt failed...");
        exit(EXIT_FAILURE);
      }
      if (holdings_parse[y] == 0) {    // fork to actual work process
        scan_holdings_parse(x+y);
        exit(EXIT_SUCCESS);
      }
    }	// end FOR(Y...
    for (y=0;y<MAXPARA;y++) {
      waitpid(holdings_parse[y],NULL,0);       // wait for thread to return
    }	// end FOR(Y...
  }     // end FOR(x...
  
  if (debug) puts("Processing the rem stuff");
  for (x=(num_holdings-rem);x<num_holdings;x+=MAXPARA) {
    for (y=0;y<rem;y++) {
      holdings_parse[y] = fork();
      if (holdings_parse[y] == -1) {
        perror("fork attempt failed...");
        exit(EXIT_FAILURE);
      }
      if (holdings_parse[y]==0) {      // fork to actual work process
        scan_holdings_parse(x+y);
        exit(EXIT_SUCCESS);
      }
    }	// end FOR(Y...
    for (y=0;y<rem;y++) {
      waitpid(holdings_parse[y],NULL,0);       // wait for thread to return
    }	// end FOR(Y...
  }	// end FOR(x...

  return(EXIT_SUCCESS);
}
