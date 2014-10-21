// KirshenbaumBands.c 
/* Kirshenbaum Bands are similar to Bollinger Bands, in that they measure market volatility. 
 * However, they use Standard Error of linear regression lines of the Close. 
 * This has the effect of measuring volatility around the current trend, 
 * instead of measuring volatility for changes in trend.
 * 
 * Parms:  Sym [Periods(20)] [Coefficient(1.75)]
 * compile:  gcc -Wall -O2 -ffast-math -o KirshenbaumBands KirshenbaumBands.c `mysql_config --include --libs`
 */

#define		MAX_PERIODS	200
#define		EMA_PERIODS	20

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<errno.h>
#include	<ctype.h>

MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];

#include        "../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"

int	main(int argc, char *argv[]) {
  MYSQL_FIELD *field;
  unsigned long	*lengths;
  char	query[1024];
  char	Sym[32];
  int	num_rows,StartRow,n;
  int	EMA_Periods=EMA_PERIODS;
  int	bad_row=0;
  double	coeff=1.75;
  double	oldema=0.0;
  double	ema=0.0;
  double	alpha,sum;
  double	a,b,d,x,y,sum_x,sum_y,sum_xy,average_x,average_y,average_xy,variance_x;
  double	linear_regression_line,covariance;
  double	sum_of_the_squared_errors,average_of_the_squared_errors,se=0;
  double	kbsup=0,kbinf=0;

  if (argc == 1 || argc>4) {
    printf("Usage:  %s Sym [Periods(20)] [coeff(1.75)]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
 // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) {
    for (n=0;n<strlen(argv[1]);n++) Sym[n]=toupper(argv[1][n]);
    valid_sym(Sym);
  }
  if (argc >= 3) EMA_Periods=strtof(argv[2],NULL);
  if (argc == 4) coeff=strtof(argv[3],NULL);
  
  // query database for required values
  sprintf(query,"select day_close,date from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol %s\n",Sym);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (MAX_PERIODS+EMA_Periods+1)) {
    printf("Not enough rows found to process Kirshenbaum Bands for %d periods\n",EMA_Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

  // good data retrieved, begin processing
  StartRow=num_rows-(MAX_PERIODS+EMA_Periods-1);
  // calc starting SMA for EMA
  mysql_data_seek(result, StartRow);
  sum=0.0;
  bad_row=0;
  for (x=0;x<EMA_Periods;x++) {
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { bad_row++; break; }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { bad_row++; break; }
    if (row[0] == NULL) { bad_row++; break; }
    sum += strtof(row[0], NULL);
  }
  if (bad_row>0) { 
    fprintf(stderr,"0 NULL data found, aborting process\n"); 
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  oldema = ema = sum/EMA_Periods;
  
  
  // calc EMA
  alpha = (double)(2 / (double)(EMA_Periods + 1));
  StartRow += EMA_Periods;
  bad_row=0;
  while (StartRow < num_rows) {
    mysql_data_seek(result, StartRow);
    row=mysql_fetch_row(result);
    if(row==NULL) { bad_row++; break; }
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { bad_row++; break; }
    if (row[0] == NULL) { bad_row++; break; }
    ema = alpha * (strtof(row[0],NULL) - oldema)  + oldema;
    oldema=ema;

// Begin Standard Error processing
    mysql_data_seek(result, StartRow-EMA_Periods+1);
    sum_x = sum_y = sum_xy = 0;
    for(n = num_rows-EMA_Periods; n < num_rows; n++) {
      row=mysql_fetch_row(result);
    // error check for nulls
      if(row==NULL) { bad_row++; break; }
      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { bad_row++; break; }
      y = strtof(row[0],NULL);	// argv[3]
      x = n;
      sum_x += x;
      sum_y += y;
      sum_xy += (x * y);
    }	// end For
  if (bad_row>0) { 
    fprintf(stderr,"1 NULL data found, aborting process in loop %d of %d\n",n,num_rows); 
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

    average_x = sum_x /EMA_Periods;
    average_y = sum_y / EMA_Periods;
    average_xy = sum_xy / EMA_Periods;
    // Calculate the covariance(x,y)
    covariance = average_xy - average_x * average_y;
    // Calculate the variance of (x) and (y)
    variance_x=0;
    for(n = num_rows-EMA_Periods; n < num_rows; n++) {
      variance_x += pow((n-average_x),2);
    }
    variance_x /= EMA_Periods;
    // Calculate the linear regression coefficients
    a = covariance / variance_x;
    b = average_y - (a * average_x);
    // Calculate the Standard Error
    mysql_data_seek(result, StartRow-EMA_Periods+1);
    sum_of_the_squared_errors = 0;
    for(n = num_rows-EMA_Periods; n < num_rows; n++) {
      row=mysql_fetch_row(result);
      // error check for nulls
      if(row==NULL) { print_error(mysql,"4 Oops!  null data found\n"); }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { print_error(mysql,"5 Oops!  null data found\n"); }
      // Calculate the linear regression line value
      linear_regression_line = (a * n) + b;
      d = strtof(row[0],NULL) - linear_regression_line;
      sum_of_the_squared_errors += pow(d,2);
    }	// end For

    // Calculate the average of the squared errors
    average_of_the_squared_errors = sum_of_the_squared_errors / EMA_Periods;
    // Calculate the standard error
    se = sqrt(average_of_the_squared_errors);
    
    // Kirshenbaum Band Sup is equal to the value of the EMA + standard error
    kbsup = ema + (coeff * se);
    // Kirshenbaum Band Inf is equal to the value of the  EMA - standard error
    kbinf = ema - (coeff * se);

    printf("[%s] =\t%.4f\t%.4f\t%.4f\n",row[1], ema, kbsup, kbinf);
    StartRow++;
  } // end While

  if (bad_row>0) { 
    fprintf(stderr,"6 NULL data found, aborting process\n"); 
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
