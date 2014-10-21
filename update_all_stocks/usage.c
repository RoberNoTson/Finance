/* usage.c
 * part of UPDATE_ALL_STOCKS
 */
#include	"update_all_stocks.h"

void	Usage(char *prog) {
  printf("Usage:  %s [force]\n \
  \tUpdate the beancounter stockinfo and stockprice with today's (or most recent) data\n\tfor all active stock symbols.\n \
  \tSupplying the \"force\" parm forces the data to be updated,\n\tcoercing the Date to today, but only if the market is closed.\n\tUse \"force\" with caution!\n", prog);
  exit(EXIT_FAILURE);
}

