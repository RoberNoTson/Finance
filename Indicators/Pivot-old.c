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

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char qDate[12];
  float	price_high, price_low, price_close,tp;

#include	"Includes/print_error.inc"
#include	"Includes/valid_date.inc"
#include	"Includes/valid_sym.inc"


// query MySQL for Sym on Date to set high, low, close prices
float get_tp(char *Sym) {
  char query[200];
  MYSQL_FIELD *field;
  unsigned long	*lengths;

  // get the prices
  sprintf(query, "select day_high, day_low, day_close, date from stockprices where symbol = \"%s\" and date < \"%s\" order by date",Sym,qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  mysql_data_seek(result,mysql_num_rows(result)-1);
  row=mysql_fetch_row(result);
  if (row==NULL) {	// no results returned
    if (mysql_errno(mysql)) {	// an error was reported
      print_error(mysql, "fetch_row failed");
    }
  }
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
  price_high = strtof(row[0], NULL);
  price_low = strtof(row[1], NULL);
  price_close = strtof(row[2], NULL);
  mysql_free_result(result);
  return((price_high + price_low + price_close) / 3);
}

int main(int argc, char *argv[]) {
  time_t t;
  struct tm *TM;
  float R1,R2,S1,S2;
  
  // connect to the database
  #include "Includes/beancounter-conn.inc"
  
  // parse cli parms
  if (argc == 1 || argc > 3) {
    printf("Usage:  %s Sym {[yyyy-mm-dd] | [##]}\n", argv[0]);
    exit(EXIT_FAILURE);
  }
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
    } else {  // number of rows to print
      printf("Function not installed yet\n");
      exit(EXIT_FAILURE);
    }
  }  
  valid_sym(argv[1]);
  valid_date(argv[1]);

  // calculate the TP
  tp = get_tp(argv[1]);
  R1 = (tp*2)-price_low;
  S1 = (tp*2)-price_high;
  R2 = (tp-S1)+R1;
  S2 = tp-(R1-S1);
  printf("[%s] =\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n",qDate,tp,S1,S2,R1,R2);
  
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
