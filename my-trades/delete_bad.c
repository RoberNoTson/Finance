/* delete_bad.c
 * part of my-trades
 */
#include "my-trades.h"
int	delete_bad(char *Sym) {
      char query[200];
      sprintf(query,"delete from TRADES where SYMBOL = \"%s\"",Sym);
      return mysql_query(mysql,query);
}
