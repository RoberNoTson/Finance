/* freeallmem.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"
int	FreeAllMem(void) {
//  if (curl) {
    if (debug) puts("freeing CURL memory");
//    curl_easy_cleanup(curl);
    curl_global_cleanup();
//  }
  if (chunk.memory) {
    if (debug) puts("freeing chunk");
    free(chunk.memory);
  }
  // finished with the database, close everything and free the memory
  return(0);
}
