// EnhPivot.c - calculate Pivot values based on Larry Levin 
// Usage:  Pivot SYM {[yyyy-mm-dd]}
/* use CURRENT_DATE unless a 'yyyy-mm-dd' passed as cli parm
 * print the most current data unless a date is passed
 * The calculation for the new day are calculated from the High (H), low (L) and close (C) of the previous day.
 * Pivot point = P = (H + L + C)/3
 * First area of resistance = R1 = 2P - L
 * First area of support = S1 = 2P - H
 * Second area of resistance = R2 = 3P - 2L
 * Second area of support = S2 = 3P - 2H
 * Third resistance = R3 = 2P + H - 2L
 * Third support = S3 = 2P + L - 2H
 * 
 * compile: gcc -Wall -O2 -ffast-math -o EnhPivot EnhPivot.c `mysql_config --include --libs`
 */

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char qDate[12];

#include	"../Includes/print_error.inc"
//#include	"../Includes/valid_date.inc"
#include	"../Includes/valid_sym.inc"

void Usage(char * argv[]) {
    printf("Usage:  %s Sym {[yyyy-mm-dd]}\n", argv[0]);
    exit(EXIT_FAILURE);
  }

int main(int argc, char *argv[]) {
  time_t t;
  struct tm *TM;
  float R1,R2,R3,S1,S2,S3,PP;
  float	CurHigh, CurLow, CurClose;
  char query[200];
  unsigned long	*lengths;
  
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  
  // parse cli parms
  if (argc == 1) { Usage(&argv[0]); }
  if (argc == 2) {
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
    } else {  Usage(&argv[0]); }
  }  
  if (argc > 3) { Usage(&argv[0]); }
  valid_sym(argv[1]);
//  valid_date(argv[1]);

  // calculate the Pivot Point
  // get the prices
  strcpy(query, "select day_high, day_low, day_close, date from stockprices where symbol = \"");
  strcat(query, argv[1]);
  strcat(query, "\" and date <= \"");
  strcat(query, qDate);
  strcat(query, "\" order by date desc limit 1");
// query processing
  if (mysql_query(mysql,query)) {
    print_error(mysql, "Failed to query database");
  }
  // save the query results
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) {	// no results returned, 
      print_error(mysql, "store_results failed");
  } 
  // fetch the last row
  
  mysql_data_seek(result,mysql_num_rows(result)-1);
  row=mysql_fetch_row(result);
  if (row==NULL) {	// no results returned
    if (mysql_errno(mysql)) {	// an error was reported
      print_error(mysql, "fetch_row failed");
    }
  }
  // check for nulls
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
  // extract data
  CurHigh = strtof(row[0], NULL);
  CurLow = strtof(row[1], NULL);
  CurClose = strtof(row[2], NULL);
  mysql_free_result(result);
  PP = (CurHigh + CurLow + CurClose) / 3;
  R1 = (PP*2)-CurLow;
  S1 = (PP*2)-CurHigh;
  R2 = (PP*3)-(CurLow*2);
  S2 = (PP*3)-(CurHigh*2);
  R3 = (PP*2)+CurHigh-(CurLow*2);
  S3 = (PP*2)+CurLow-(CurHigh*2);
  printf("[%s] =\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",qDate,S3,S2,S1,PP,R1,R2,R3);  
  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
