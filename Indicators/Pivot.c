// Pivot.c - calculate Pivot values
// Usage:  Pivot SYM {[yyyy-mm-dd] | [##]}
/* use CURRENT_DATE unless a 'yyyy-mm-dd' passed as cli parm
 * print only the most current data unless a number is passed as cli parm
 * The calculation for the new day are calculated from the High (H), low (L) and close (C) of the previous day.
 * Pivot point = P = (H + L + C)/3
 * First area of resistance = R1 = 2P - L
 * First area of support = S1 = 2P - H
 * Second area of resistance = R2 = (P -S1) + R1
 * Second area of support = S2 = P - (R1 - S1)
 * 
 * compile: gcc -Wall -O2 -ffast-math -o Pivot Pivot.c `mysql_config --include --libs`
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
  char qDate[12];

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_date.inc"
#include	"../Includes/valid_sym.inc"


int main(int argc, char *argv[]) {
  time_t t;
  struct tm *TM;
  int	x;
  float R1,R2,S1,S2,PP;
  float	CurHigh, CurLow, CurClose;
  char query[200];
  char	Sym[32];
  MYSQL_FIELD *field;
  unsigned long	*lengths;
  
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  
  // parse cli parms
  if (argc == 1) {
    printf("Usage:  %s Sym {[yyyy-mm-dd] | [##]}\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc == 2) {
    for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
    t = time(NULL);
    TM = localtime(&t);
    if (TM == NULL) {
      perror("localtime");
      exit(EXIT_FAILURE);
    }
    if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
      fprintf(stderr, "strftime returned 0");
      exit(EXIT_FAILURE);
    }
  }
  if (argc == 3) {
    // is it a date?
    if (strlen(argv[2]) == 10) {	// yes, process it
      strcpy(qDate, argv[2]);
    } else {  // number of rows to print
      printf("Function not installed yet\n");
      exit(EXIT_FAILURE);
    }
  }  
  if (argc > 3) {
    printf("Usage:  %s Sym {[yyyy-mm-dd] | [##]}\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  valid_sym(Sym);
  valid_date(Sym);

  // calculate the Pivot Point
  // get the prices
  sprintf(query,"select day_high, day_low, day_close, date from stockprices where symbol = \"%s\" and date <= \"%s\" order by date",Sym,qDate);
  // query processing
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  // save the query results
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  // fetch the last row
  mysql_data_seek(result,mysql_num_rows(result)-1);
  row=mysql_fetch_row(result);
  if (row==NULL) print_error(mysql, "fetch_row failed");
  // check for nulls
  mysql_field_seek(result,0);
  field = mysql_fetch_field(result);
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  // extract data
  CurHigh = strtof(row[0], NULL);
  CurLow = strtof(row[1], NULL);
  CurClose = strtof(row[2], NULL);
  mysql_free_result(result);
  PP = (CurHigh + CurLow + CurClose) / 3;
  R1 = (PP*2)-CurLow;
  S1 = (PP*2)-CurHigh;
  R2 = (PP-S1)+R1;
  S2 = PP-(R1-S1);
  printf("[%s] =\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n",qDate,PP,S1,S2,R1,R2);
  
  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
