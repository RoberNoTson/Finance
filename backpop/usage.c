/* usage.c
 * part of watchlist_monitor
 */
#include "backpop.h"
void	Usage(char *prog) {
    printf("Usage:  %s Sym [yyyy-mm-dd]\n \
    Supplying today\'s date forces an update from YahooQuote if the market is closed.\n \
    Any other date (or no date) will only update existing price history\n", prog);
    exit(EXIT_FAILURE);
}
