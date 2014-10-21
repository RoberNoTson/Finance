/* usage.c
 * part of my-trades
 */
#include "my-trades.h"
int	Usage(char *Sym) {
  printf("Usage:  %s [yyyy-mm-dd]\n", Sym);
  exit(EXIT_FAILURE);
}
