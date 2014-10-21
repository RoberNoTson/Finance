/* sighandlers.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"
int SigHandler (int signum) {
  struct	stat FileInfo;
  switch(signum) {
    case SIGINT: {
      if (debug) printf ("Program ended by Ctl-C interrupt\n");
      Cleanup(EXIT_SUCCESS);
      break;
    }
    case SIGUSR1: {
      // reload the config file
      if ((stat(config_file_name, &FileInfo)<0) || (ParseConfig(prog)>0)) {
	printf("Error reading config file %s\n",config_file_name);
	Cleanup(EXIT_FAILURE);
      }
      printf ("Reloaded config file\n");
      break;
    }
    default:
      break;
  }
  return(EXIT_SUCCESS);
}
