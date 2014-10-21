/* HV_check.c
 * part of my-trades
 */
#include	"./my-trades.h"

int	HV_check(char *Sym) {
  char	query[1024];
  int	num_rows;
  unsigned long	*lengths;

    sprintf(query, "select avg_volume,volume from stockinfo i, stockprices s \
      where s.symbol = \"%s\" and s.symbol=i.symbol order by date desc limit 1",Sym);
    if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query database"); }
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
    if ((num_rows=mysql_num_rows(result)) != 1) { print_error(mysql, "Volume query returned 0 rows, aborting"); }
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Volume NULL data found, skipping %s",Sym); return EXIT_FAILURE; }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Volume NULL data found, skipping %s",Sym); return EXIT_FAILURE; }
    if (!lengths[1]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Volume NULL data found, skipping %s",Sym); return EXIT_FAILURE; }
    if (strtof(row[1],NULL) > strtof(row[0],NULL)) {	// got high volume for previous day
      sprintf(query,"update TRADES set HV = true where SYMBOL = \"%s\"",Sym);
      if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update symbol into temp database"); }
    } // end if
    mysql_free_result(result);    

    return EXIT_SUCCESS;
}
