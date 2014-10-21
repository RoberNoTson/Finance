// StandardError.c
/* Standard Error is a statistical measure of volatility. 
 * Standard Error is typically used as a component of other indicators, 
 * rather than as a stand-alone indicator. 
 * For example, Kirshenbaum Bands are calculated by adding a security's Standard Error 
 * to an exponential moving average.
 * Calculate the L-Period linear regression line, using today's Close as the endpoint of the line.
 * Calculate d1, d2, d3, ..., dL as the distance from the line to the Close 
 * of each bar which was used to derive the line. 
 * That is, d(i) = Distance from Regression Line to each bar's Close.
 * Average of squared errors (AE) = (d1² + d2² + d3² + ... + dL²) / L
 * Standard Error = Square Root of AE
 * 
 * Parms: Sym [Periods(20)] [close(default)|open|high|low|volume|change]
 * compile: gcc -Wall -O2 -ffast-math -o StandardError StandardError.c `mysql_config --include --libs`
 */

#define         MAX_PERIODS 200
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <time.h>
#include	<ctype.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char  qDate[12];

#include        "../Includes/print_error.inc"
#include        "../Includes/valid_sym.inc"
#include        "../Includes/valid_date.inc"

int main(int argc, char *argv[]) {
  int	Periods=20;
  int	qType=3;
  int	num_rows,n,StartRow;
  char	query[200];
  char	Sym[32];
  double	a,b,d,x,y,sum_x,sum_y,sum_xy,average_x,average_y,average_xy,variance_x;
  double	linear_regression_line,covariance;
  double	sum_of_the_squared_errors,average_of_the_squared_errors,standard_error;
//  MYSQL_FIELD *field;
  time_t t;
  struct tm *TM;
  unsigned long *lengths;

  // parse cli parms
  if (argc == 1 || argc > 4) {
    printf("Usage:  %s Sym [periods [close(default)|open|high|low|volume|change]]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) {
    for (n=0;n<strlen(argv[1]);n++) Sym[n]=toupper(argv[1][n]);
    valid_sym(Sym);
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
  if (argc >= 3) {
    // is it a date?
    if (strlen(argv[2]) == 10) {        // yes, process it
      strcpy(qDate, argv[2]);
    } else {  // number of rows to print
        Periods=atoi(argv[2]);
    }
  }
  if (argc == 4) {
    if (strcasecmp(argv[3], "open") == 0) qType=0;
    else if (strcasecmp(argv[3], "low") == 0) qType=1;
    else if (strcasecmp(argv[3], "high") == 0) qType=2;
    else if (strcasecmp(argv[3], "close") == 0) qType=3;
    else if (strcasecmp(argv[3], "change") == 0) qType=4;
    else if (strcasecmp(argv[3], "volume") == 0) qType=5;
    else {
      printf("Usage:  %s Sym [periods] [close(default)|open|high|low|volume|change]\n", argv[0]);
  #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
  }

  // query all data
  sprintf(query,"select day_open,day_low,day_high,day_close,day_change,volume,date from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\"\n", Sym);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (MAX_PERIODS+Periods)) {
    printf("Not enough rows found to process Linear Regression for %d periods\n",Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

// Begin main Standard Error processing
  // Calculate the average of (x), (y) and (xy)
  StartRow=num_rows-(MAX_PERIODS+Periods);
  while (StartRow < (num_rows-Periods+1)) {
    mysql_data_seek(result, StartRow);
    sum_x = sum_y = sum_xy = 0;
    for(n = num_rows-Periods; n < num_rows; n++) {
      row=mysql_fetch_row(result);
    // error check for nulls
      if(row==NULL) { print_error(mysql,"1 Oops!  null data found\n"); }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[qType]) { print_error(mysql,"2 Oops!  null data found\n"); }
      y = strtof(row[qType],NULL);	// argv[3]
      x = n;
      sum_x += x;
      sum_y += y;
      sum_xy += (x * y);
    }	// end For
    average_x = sum_x / Periods;
    average_y = sum_y / Periods;
    average_xy = sum_xy / Periods;
    // Calculate the covariance(x,y)
    covariance = average_xy - average_x * average_y;
    // Calculate the variance of (x) and (y)
    variance_x=0;
    for(n = num_rows-Periods; n < num_rows; n++) {
      variance_x += pow((n-average_x),2);
    }
    variance_x /= Periods;
    // Calculate the linear regression coefficients
    a = covariance / variance_x;
    b = average_y - (a * average_x);
    // Calculate the Standard Error
    mysql_data_seek(result, StartRow);
    sum_of_the_squared_errors = 0;
    for(n = num_rows-Periods; n < num_rows; n++) {
      row=mysql_fetch_row(result);
      // error check for nulls
      if(row==NULL) { print_error(mysql,"4 Oops!  null data found\n"); }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[qType]) { print_error(mysql,"5 Oops!  null data found\n"); }
      // Calculate the linear regression line value
      linear_regression_line = (a * n) + b;
      d = strtof(row[qType],NULL) - linear_regression_line;
      sum_of_the_squared_errors += pow(d,2);
    }	// end For
    // Calculate the average of the squared errors
    average_of_the_squared_errors = sum_of_the_squared_errors / Periods;
    // Calculate the standard error
    standard_error = sqrt(average_of_the_squared_errors);
    printf("[%s] =\t%.4f\n",row[6],standard_error);
    StartRow++;
  } // end While

  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
