// MinInPeriod.c
/* This indicator calculates the minimum of any series of data in the last XX days or since a given date.
 * Parms:  Symbol Number|Date [Data]
 * You can specify either a number or a date. In the first case the minimum will be calculated 
 * with the last <number> days. In the second case it will be the minimum since the given date.
 * The data to use as input, if you don't specify anything, the Low price will be used by default.
 * Data options are ["Close" "High" "Low"]
 * compile:  gcc -Wall -O2 -ffast-math MinInPeriod.c -o MinInPeriod `mysql_config --include --libs`
 */

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

int Usage(char *prog) {
    printf("Display the lowest requested price value\nUsage:  %s Sym {## | yyyy-mm-dd} [Data type]\n \
    where the second arg is either a date or number of sessions to look back,\n \
    and the optional third arg is one of [Low(default), High, Close]\n", prog);
    exit(EXIT_FAILURE);
  }


int main(int argc, char *argv[]) {
  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char qDate[12];
  float Min;
  char query[200];
  char	Sym[16];
  char	ValType[16]="day_low";
  int	Periods=0;
  int	x;
  unsigned long	*lengths;

  #include	"../Includes/print_error.inc"
  #include	"../Includes/valid_sym.inc"

  memset(query,0,sizeof(query));
  // parse cli parms, if any
  if (argc<3 || argc>4) Usage(argv[0]);
  // is it a date?
  if (strlen(argv[2]) == 10) {	// yes, process it
    strcpy(qDate, argv[2]);
  } else {  // number of rows to print
    Periods=atoi(argv[2]);
  }
  if (argc==4) {
    if (strcasecmp("high",argv[3])==0) strcpy(ValType,"day_high");
    else if (strcasecmp("low",argv[3])==0) strcpy(ValType,"day_low");
    else if (strcasecmp("close",argv[3])==0) strcpy(ValType,"day_close");
    else Usage(argv[0]);
  }
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
 
  // make symbol uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  if (valid_sym(Sym)) print_error(mysql,"Aborting");

  if (strlen(qDate) == 10) {
    sprintf(query,"select min(%s) from stockprices where symbol = \"%s\" and date >= \"%s\"", \
    ValType,Sym,qDate);
  } else {
    sprintf(query,"select min(sp.%s) from (select distinct(date) as idate from stockprices \
    where symbol=\"%s\" order by date desc limit %d) as A inner join stockprices as sp \
    on DATE(sp.date) = DATE(A.idate) where sp.symbol=\"%s\" order by sp.date desc", \
    ValType,Sym,Periods,Sym);
  }
  // query the max value
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  row=mysql_fetch_row(result);
  if (row==NULL) print_error(mysql,"Unable to find any valid data");
  // check for nulls
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0]) print_error(mysql,"Null data found, abandoning process");
  Min=strtof(row[0], NULL);
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  // output the answer
  printf("%.4f\n",Min);
  exit(EXIT_SUCCESS);
}
