/* Fibonacci.c
* Calculate Fibonacci values based on 3 input values
* Usage: Fibonacci Symbol | #.#  #.#  #.#  (e.g. High, Low, Close)
* Algorithm: ((High-Low)*FibVal)+Close
* Useful for stock price estimates
* Passing a stock symbol will use the most recent high/low/close data
* 
* -6.850 -4.25 -2.618 -1.982 -1.918 -1.800	Extended ranges
* -1.0
* -0.75 -0.618 -0.5 -0.48 -0.382 -0.25	normal retracements
* 0.000 
* 0.250 0.382 0.480 0.500 0.618 0.750  	normal retracements
* 1.0
* 1.800 1.918 1.982 2.618 4.250 6.850 	Extended ranges
* 
* gcc -Wall -O2 -ffast-math Fibonacci.c -o Fibonacci `mysql_config --include --libs`
*/

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <string.h>
#include	<math.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

#include	"Includes/print_error.inc"
#include	"Includes/valid_sym.inc"


int     main(int argc, char *argv[]) {
  int x;
//  float High=0,Low=0,Close=0, diff;
  int High=0,Low=0,Close=0, diff;
  char query[200];
  char	Sym[32];
  unsigned long	*lengths;
  
  if (argc==3 || argc==1 || argc>4) {
    printf("Usage: %s Symbol | #.##(High) #.##(Low) #.##(Close)\n \
    Pass either a stock symbol or 3 numeric values\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc==4) {
    High=(int)(strtof(argv[1],NULL)*100);
    Low=(int)(strtof(argv[2],NULL)*100);
    Close=(int)(strtof(argv[3],NULL)*100);
  }
  else {
    strcpy(Sym, argv[1]);
    // connect to the database
    #include "Includes/beancounter-conn.inc"
    if (valid_sym(Sym)) print_error(mysql, "Exiting process");
    // query for the prices
    sprintf(query,"select day_high, day_low, day_close from stockprices where symbol = \"%s\" order by date desc limit 1",Sym);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed");
    if (!mysql_num_rows(result))  print_error(mysql, "No rows returned from query, aborting");
    if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql, "fetch_row failed");
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { mysql_free_result(result); print_error(mysql,"Empty column found, abandoning process"); }
    }
    // parse data
    High = (int)(strtof(row[0], NULL)*100);
    Low = (int)(strtof(row[1], NULL)*100);
    Close = (int)(strtof(row[2], NULL)*100);
    // finished with the database
    mysql_free_result(result);
    #include "Includes/mysql-disconn.inc"
  }
//  diff=fabsf(High-Low);
  diff=abs(High-Low);
  printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",(float)(Close+(-6.85*diff))/100,(float)(Close+(-4.25*diff))/100,(float)(Close+(-2.618*diff))/100,(float)(Close+(-1.982*diff))/100,(float)(Close+(-1.918*diff))/100,(float)(Close+(-1.8*diff))/100);
  printf("%.2f\n",(float)(Close+(-1.0*diff))/100);
  printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",(float)(Close+(-0.75*diff))/100,(float)(Close+(-0.618*diff))/100,(float)(Close+(-0.5*diff))/100,(float)(Close+(-0.382*diff))/100,(float)(Close+(-0.25*diff))/100);
  printf("%.2f\n",(float)Close/100);
  printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",(float)(Close+(0.25*diff))/100,(float)(Close+(0.382*diff))/100,(float)(Close+(0.5*diff))/100,(float)(Close+(0.618*diff))/100,(float)(Close+(0.75*diff))/100);
  printf("%.2f\n",(float)(Close+diff)/100);
  printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",(float)(Close+(1.8*diff))/100,(float)(Close+(1.918*diff))/100,(float)(Close+(1.982*diff))/100,(float)(Close+(2.618*diff))/100,(float)(Close+(4.25*diff))/100,(float)(Close+(6.85*diff))/100);

  exit(EXIT_SUCCESS);
}
