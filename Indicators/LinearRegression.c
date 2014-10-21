/* LinearRegression.c
 * The term "linear regression" is the same as a "least squares" or "best fit" line.
 * The linear regression value is "a * i + b". where i is the day number. a and b are coefficient values
 * Takes 2 or 3 parameters. The first is the period over which the regression 
 * is calculated. The following parameters indicate the series that are being compared. 
 * If there is only a second parameter, this parameter forms the dependent variables, 
 * while the numerical sequence is the independent parameter. 
 * If there are both a second and a third parameter, the former is the independent 
 * and the latter the dependent parameter.
 * 
 * Parms:  Sym [Period] [close(default)|open|high|low|volume|change]
 * compile:  gcc -Wall -O2 -ffast-math -o LinearRegression LinearRegression.c `mysql_config --include --libs`
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
  int	Periods=50;
  int	aType=3;
  int	bType=2;
  int	num_rows,n,StartRow;
  int	Ctr;
  char	query[200];
  char	Sym[32];
  double	a,b,x,y,sum_x,sum_y,sum_xy,average_x,average_y,average_xy,variance_x;
  double	linear_regression_line,covariance;
  time_t t;
  struct tm *TM;
  unsigned long *lengths;

  // parse cli parms
  if (argc == 1 || argc > 5) {
    printf("Usage:  %s Sym [periods [close(default)|open|high|low|volume|change] [close(default)|open|high|low|volume|change]]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) {
    for (Ctr=0;Ctr<strlen(argv[1]);Ctr++) Sym[Ctr]=toupper(argv[1][Ctr]);
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
  if (argc >= 4) {
    if (strcasecmp(argv[3], "open") == 0) aType=0;
    else if (strcasecmp(argv[3], "low") == 0) aType=1;
    else if (strcasecmp(argv[3], "high") == 0) aType=2;
    else if (strcasecmp(argv[3], "close") == 0) aType=3;
    else if (strcasecmp(argv[3], "change") == 0) aType=4;
    else if (strcasecmp(argv[3], "volume") == 0) aType=5;
    else {
      printf("Usage:  %s Sym [periods] [close(default)|open|high|low|volume|change]\n", argv[0]);
  #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
  }
  if (argc == 5) {
    if (strcasecmp(argv[4], "open") == 0) bType=0;
    else if (strcasecmp(argv[4], "low") == 0) bType=1;
    else if (strcasecmp(argv[4], "high") == 0) bType=2;
    else if (strcasecmp(argv[4], "close") == 0) bType=3;
    else if (strcasecmp(argv[4], "change") == 0) bType=4;
    else if (strcasecmp(argv[4], "volume") == 0) bType=5;
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
  
  // Calculate the average of (x), (y) and (xy)
  StartRow=num_rows-(MAX_PERIODS+Periods);
  Ctr=(num_rows-Periods+1);
  while (StartRow < Ctr) {
    mysql_data_seek(result, StartRow);
    sum_x = sum_y = sum_xy = 0;
    variance_x=0;
    for(n = num_rows-Periods; n < num_rows; n++) {
      row=mysql_fetch_row(result);
    // error check for nulls
      if(row==NULL) { print_error(mysql,"1 Oops!  null data found\n"); }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[aType]) { print_error(mysql,"2 Oops!  null data found\n"); }
      if (!lengths[bType]) { print_error(mysql,"2.5 Oops!  null data found\n"); }
      
      x = strtof(row[aType],NULL);	// argv[3]
      if (argc==5) {
	y = strtof(row[bType],NULL);
      } else {
	y = x;
	x = n;
      }
      sum_x += x;
      sum_y += y;
      sum_xy += (x * y);
    }
    average_x = sum_x / Periods;
    average_y = sum_y / Periods;
    average_xy = sum_xy / Periods;
    // Calculate the covariance(x,y)
    covariance = average_xy - average_x * average_y;
    
    // Calculate the variance of (x) and (y)
    mysql_data_seek(result, StartRow);
    for(n = num_rows-Periods; n < num_rows; n++) {
      row=mysql_fetch_row(result);
    // error check for nulls
      if(row==NULL) { print_error(mysql,"4 Oops!  null data found\n"); }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[aType]) { print_error(mysql,"5 Oops!  null data found\n"); }
      
	x = strtof(row[bType],NULL);
      if (argc==5) {
	if (!lengths[bType]) { print_error(mysql,"5.5 Oops!  null data found\n"); }
	x = strtof(row[bType],NULL);
      } else {
	y = x;
	x = n;
      }
      variance_x += pow((x - average_x),2);
    }
    variance_x /= Periods;
    // Calculate the linear regression coefficients
    a = covariance / variance_x;
    b = average_y - (a * average_x) + a*((Ctr-1)-StartRow);
    // Calculate the linear regression line value
    linear_regression_line = (a * (num_rows-1)) + b - a*((Ctr-1)-StartRow);
    printf("[%s] =\t%.4f\t%.4f\t%.4f\n",row[6],linear_regression_line,a,b);
    StartRow++;
  } // end While

// finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
