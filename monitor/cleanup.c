/* cleanup.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"
void	Cleanup(int rc) {
  FreeAllMem();
  puts("Program completed, press 'Q' to Quit");
  exit(rc);
}
