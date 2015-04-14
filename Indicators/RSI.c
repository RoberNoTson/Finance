// RSI.c
/* The standard RSI is the RSI 14 days using CLOSE prices
 * 
 * Parms: Sym [Periods]
 * compile: gcc -Wall -O2 -ffast-math RSI.c -o RSI `mysql_config --include --libs`
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

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"

int main(int argc, char * argv[]) {
  char query[200];
  char	Sym[32];
  float	CurClose,PrevClose,diff,rsi,rs=0.0;
  float	won_points=0.0,lost_points=0.0;
  int Periods=14,x,num_rows;
  MYSQL_FIELD *field;
  unsigned long *lengths;

  // parse cli parms
  if (argc==1 || argc>3) {
    printf("Usage:  %s Sym [periods(14)]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  valid_sym(Sym);
  if (argc == 3) Periods=atoi(argv[2]);
//  sprintf(query,"select day_close, previous_close, date from stockprices where symbol = \"%s\" order by date",Sym);
  sprintf(query,"(select day_close, previous_close, date from stockprices where symbol = \"%s\" order by date desc limit %d) order by date",Sym,Periods+1);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);       // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\"\n", Sym);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (Periods)) {
    printf("Not enough rows found to process RSI for %d periods\n",Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

// calculate the requested RSI
  mysql_data_seek(result, num_rows-Periods);
  row=mysql_fetch_row(result);
  // check for nulls
  if(row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  mysql_field_seek(result,0);
  field = mysql_fetch_field(result);
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  CurClose=strtof(row[0],NULL);
  PrevClose=strtof(row[1],NULL);
  diff = CurClose - PrevClose;
  // Add the won points
  if (diff > 0) won_points += diff;
  // Add the lost points
  if (diff < 0) lost_points -= diff;
  PrevClose=CurClose;
  for(x = num_rows-Periods+1; x < num_rows; x++) {
    row=mysql_fetch_row(result);
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    CurClose=strtof(row[0],NULL);
    diff = CurClose - PrevClose;
    // Add the won points
    if (diff > 0) won_points += diff;
    // Add the lost points
    if (diff < 0) lost_points -= diff;
    PrevClose=CurClose;
  }
  if (lost_points != 0) rs = won_points / lost_points;
  rsi = 100 - (100 / (1 + rs));
  printf("[%s] =\t%.4f\n",row[2],rsi);
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
