// Trend.c
/* check for valid trends with:
 * Up:  New tops each day, each bottom is higher, the two first candle are white
 * Down: New bottoms each day, each bottom is lower, etc.
 * Returns:  1(up), -1(dn), 0(no trend)
 * Parms:  Sym
 * compile: gcc -Wall -O2 -ffast-math Trend.c -o Trend `mysql_config --include --libs`
 */

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

#include	"Includes/print_error.inc"
#include	"Includes/valid_sym.inc"

int main(int argc, char *argv[]) {
  MYSQL_FIELD *field;
  char query[200];
  char	Sym[16];
  unsigned long	*lengths;
  float	CurHigh,PrevHigh,Prev2High,CurLow,PrevLow,Prev2Low,PrevClose,Prev2Close,PrevOpen,Prev2Open;
  int	num_rows,x;
  int	verbose=0;
  
  // parse cli parms
  if (argc == 1 || argc > 3) {
    printf("Usage:  %s Sym [verbose]\n\tReturns  rc=1 when up trend, rc=0 for flat, rc=-1(255) when down\n\t\"Sym\" \"Up\",\"Flat\", or \"Down\" if verbose specified\n\tCLI usage example:  if (( `./Trend $Symbol; echo $?` )); then echo Up; fi\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc == 3 && (!strcmp(argv[2],"verbose"))) verbose++;
  // make symbol uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }

  // connect to the database
  #include "Includes/beancounter-conn.inc"
  valid_sym(Sym);
  sprintf(query,"select day_high,day_low,day_open,day_close from stockprices where symbol = \"%s\" order by date asc",Sym);
    if (mysql_query(mysql,query)) {
      print_error(mysql, "Failed to query database");
    }
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) {	// no results returned, 
      print_error(mysql, "store_results failed");
    }
    num_rows=mysql_num_rows(result);
    mysql_data_seek(result, num_rows-3);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[3])  { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[3] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    Prev2High=strtof(row[0],NULL);
    Prev2Low=strtof(row[1],NULL);
    Prev2Open=strtof(row[2],NULL);
    Prev2Close=strtof(row[3],NULL);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[3])  { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[3] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    PrevHigh=strtof(row[0],NULL);
    PrevLow=strtof(row[1],NULL);
    PrevOpen=strtof(row[2],NULL);
    PrevClose=strtof(row[3],NULL);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[3])  { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[3] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    CurHigh=strtof(row[0],NULL);
    CurLow=strtof(row[1],NULL);
  
    mysql_free_result(result);
  // finished with the database
  #include "Includes/mysql-disconn.inc"
    
    if ( // New tops each day
	CurHigh >= PrevHigh &&
	PrevHigh > Prev2High &&
	// Each bottom is higher
	CurLow > PrevLow &&
	PrevLow > Prev2Low &&
	// The two first candle are white
	Prev2Close > Prev2Open &&
	PrevClose > PrevOpen ) {
	if (verbose) printf("%s Up\n",Sym);
	exit(1);
    } else if( // New bottom each day
	CurLow <= PrevLow &&
	PrevLow < Prev2Low &&
	// Each top is lower
	CurHigh < PrevHigh &&
	PrevHigh < Prev2High &&
        // The two first candle are black
        Prev2Close < Prev2Open &&
	PrevClose < PrevOpen ) {
	if (verbose) printf("%s Down\n",Sym);
	exit(-1);
    } else {
      if (verbose) printf("%s Flat\n",Sym);
      exit(0);
    }
}
