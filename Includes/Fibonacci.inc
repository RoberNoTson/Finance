// Fibonacci.inc - provide Fibonacci numbers for a stock as a struct of values.
// Usage:  Fibonnaci(Symbol, &FibStruct);
/* Calculate Fibonacci values based on 3 input values
* Algorithm: ((High-Low)*FibVal)+Close
* Useful for stock price estimates
* Passing a stock symbol will use the most recent high/low/close data
*
* Fibonacci mulipliers 
* -6.850 -4.25 -2.618 -1.982 -1.918 -1.800      Extended ranges
* -1.0                                  =Close - (High - Low) ; same as Pivot Point
* -0.75 -0.618 -0.5 -0.48 -0.382 -0.25  normal retracements
* 0.000                                 Close value
* 0.250 0.382 0.480 0.500 0.618 0.750   normal retracements
* 1.0                                   =Close + (High - Low) ; same as Pivot Point
* 1.800 1.918 1.982 2.618 4.250 6.850   Extended ranges
*/

/* requires this variable in the calling program:
  double Fibstruct[25]
*/

#ifndef _FIB_INC
#define _FIB_INC 1

#define _XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <time.h>
#include        <math.h>


int Fibonacci(char *Sym) {
  MYSQL_RES *result;
  MYSQL_ROW row=0;
  MYSQL_FIELD *field;
  unsigned long *lengths;
  int High=0,Low=0,Close=0, diff;
  char query[200];
  sprintf(query, "select day_high,day_low,day_close from stockprices where symbol = \"%s\" order by date desc limit 1",Sym);
  if (mysql_query(mysql,query)) { puts("Failed to query database in Fib.inc"); return(EXIT_FAILURE); }
    if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed");
    if (!mysql_num_rows(result))  print_error(mysql, "No rows returned from query, aborting");
    if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql, "fetch_row failed");

  // check for nulls
  mysql_field_seek(result,0);
  field = mysql_fetch_field(result);
  lengths=mysql_fetch_lengths(result);

  if (!lengths[0]) { puts("Null data found, abandoning process Fib.inc"); mysql_free_result(result); return(EXIT_FAILURE); }
  if (row[0] == NULL) { puts("Null data found, abandoning process Fib.inc"); mysql_free_result(result); return(EXIT_FAILURE); }
  if (!lengths[1]) { puts("Null data found, abandoning process Fib.inc"); mysql_free_result(result); return(EXIT_FAILURE); }
  if (row[1] == NULL) { puts("Null data found, abandoning process Fib.inc"); mysql_free_result(result); return(EXIT_FAILURE); }
  if (!lengths[2]) { puts("Null data found, abandoning process Fib.inc"); mysql_free_result(result); return(EXIT_FAILURE); }
  if (row[2] == NULL) { puts("Null data found, abandoning process Fib.inc"); mysql_free_result(result); return(EXIT_FAILURE); }

  // parse data
  High = (int)(strtof(row[0], NULL)*100);
  Low = (int)(strtof(row[1], NULL)*100);
  Close = (int)(strtof(row[2], NULL)*100);
  // finished with the database
  mysql_free_result(result);
  diff=abs(High-Low);
  FibStruct[0] = (Close+(-6.85*diff))/100;	// Extended Ranges
  FibStruct[1] = (Close+(-4.25*diff))/100;	// Extended Ranges
  FibStruct[2] = (Close+(-2.618*diff))/100;	// Extended Ranges
  FibStruct[3] = (Close+(-1.982*diff))/100;	// Extended Ranges
  FibStruct[4] = (Close+(-1.918*diff))/100;	// Extended Ranges
  FibStruct[5] = (Close+(-1.8*diff))/100;
  FibStruct[6] = (Close+(-1.0*diff))/100;	// same as Pivot Point
  FibStruct[7] = (Close+(-0.75*diff))/100;
  FibStruct[8] = (Close+(-0.618*diff))/100;
  FibStruct[9] = (Close+(-0.5*diff))/100;
  FibStruct[10] = (Close+(-0.382*diff))/100;
  FibStruct[11] = (Close+(-0.25*diff))/100;
  FibStruct[12] = Close/100;			// prev Close value
  FibStruct[13] = (Close+(0.25*diff))/100;
  FibStruct[14] = (Close+(0.382*diff))/100;
  FibStruct[15] = (Close+(0.5*diff))/100;
  FibStruct[16] = (Close+(0.618*diff))/100;
  FibStruct[17] = (Close+(0.75*diff))/100;
  FibStruct[18] = (Close+(1.0*diff))/100;	// same as Pivot Point
  FibStruct[19] = (Close+(1.8*diff))/100;
  FibStruct[20] = (Close+(1.918*diff))/100;	// Extended Ranges
  FibStruct[21] = (Close+(1.982*diff))/100;	// Extended Ranges
  FibStruct[22] = (Close+(2.618*diff))/100;	// Extended Ranges
  FibStruct[23] = (Close+(4.25*diff))/100;	// Extended Ranges
  FibStruct[24] = (Close+(6.85*diff))/100;	// Extended Ranges

  return(EXIT_SUCCESS);
}
#endif	/* !_FIB_INC */
