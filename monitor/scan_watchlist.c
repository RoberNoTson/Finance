/* scan_watchlist.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"

int	scan_watchlist(int num_stocks) {
  int	x,y,rem;
  pthread_t	watchlist_parse[MAXPARA];

  rem = num_stocks % MAXPARA;
  
  for (x=0;x<(num_stocks-rem);x+=MAXPARA) {
    for (y=0;y<MAXPARA;y++) {
      watchlist_parse[y] = fork();
      if (watchlist_parse[y] == -1) {
	perror("fork attempt failed...");
	exit(EXIT_FAILURE);
      }
      if (watchlist_parse[y] == 0) {	// fork to actual work process
	scan_watchlist_parse(x+y);
	exit(EXIT_SUCCESS);
      }
    }
    
    for (y=0;y<MAXPARA;y++) {
      waitpid(watchlist_parse[y],NULL,0);	// wait for thread to return
    }    
  }	// end FOR(x...
  
  if (debug) puts("Processing the rem stuff");
  for (x=(num_stocks-rem);x<num_stocks;x+=MAXPARA) {
    for (y=0;y<rem;y++) {
      watchlist_parse[y] = fork();
      if (watchlist_parse[y] == -1) {
	perror("fork attempt failed...");
	exit(EXIT_FAILURE);
      }
      if (watchlist_parse[y]==0) {	// fork to actual work process
	scan_watchlist_parse(x+y);
	exit(EXIT_SUCCESS);
      }
    }
    for (y=0;y<rem;y++) {
      waitpid(watchlist_parse[y],NULL,0);	// wait for thread to return
    }
  }
  
  return(EXIT_SUCCESS);
}
