// EMA.c -- Exponential Moving Average
/* An exponential moving average gives more importance to recent prices
 * The first argument is the period used to calculate the average.
 * The second argument is optional. It can be used to specify another
 * stream of input data for the average instead of the close prices.
 * The starting point of the moving average is computed by the SMA of the given period.
 * alpha = 2 / ( Periods(20) + 1 )
 * EMA[i] = EMA[i-1] + (alpha * (close-price - EMA[i-1]))
 * Parms: Periods(20) Type(close)
 * compile:  gcc -Wall -O2 -ffast-math EMA.c -o EMA `mysql_config --include --libs`
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

#include        "../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"
  
int Usage(char * arg) {
    printf("Usage:  %s Sym [Periods(20)] [close(default)|open|high|low|volume|change]\n", arg);
    exit(EXIT_FAILURE);
}
  
int	main(int argc, char * argv[]) {
  char	query[200];
  char	Sym[32];
  int	num_rows,StartRow,x;
  int	EMA_Periods=EMA_PERIODS;
  int	qType=3;
  int	bad_row=0;
  double	oldema=0.0;
  double	ema=0.0;
  double	alpha,sum;
  MYSQL_FIELD *field;
  unsigned long	*lengths;

  
  // cli parms
  if (argc == 1) {
    Usage(argv[0]);
  }
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) {
    for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
    valid_sym(Sym);
  }
  if (argc >= 3) {
    EMA_Periods=atoi(argv[2]);
  }
  if (argc == 4) {
    if (strcasecmp(argv[3], "open") == 0) qType=0;
    else if (strcasecmp(argv[3], "low") == 0) qType=1;
    else if (strcasecmp(argv[3], "high") == 0) qType=2;
    else if (strcasecmp(argv[3], "close") == 0) qType=3;
    else if (strcasecmp(argv[3], "change") == 0) qType=4;
    else if (strcasecmp(argv[3], "volume") == 0) qType=5;
    else {
      Usage(argv[0]);
    }
  }
  if (argc > 4) {
    Usage(argv[0]);
  }

  // query all data
  sprintf(query,"select day_open,day_low,day_high,day_close,day_change,volume,date from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\"\n", Sym);
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  if (num_rows < MAX_PERIODS+EMA_Periods+1)  {
    printf("Too few rows found for symbol \"%s\"\n", Sym);
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

  // start of processing
  StartRow=num_rows-(MAX_PERIODS+EMA_Periods-1);
  // calc starting SMA
  mysql_data_seek(result, StartRow);
  sum=0.0;
  for (x=0;x<EMA_Periods;x++) {
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { bad_row++; break; }
    mysql_field_seek(result,qType);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[qType]) { bad_row++; break; }
    if (row[qType] == NULL) { bad_row++; break; }
    sum += strtof(row[qType], NULL);
  }
  if (bad_row>0) { 
    fprintf(stderr,"2 NULL data found, aborting process\n"); 
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  oldema = ema = sum/EMA_Periods;
  printf("[%s] =\t%.4f\n",row[6],ema);

  // calc EMA
  alpha = (double)(2 / (double)(EMA_Periods + 1));
  StartRow += EMA_Periods;
  bad_row=0;
  while (StartRow < num_rows) {
    mysql_data_seek(result, StartRow);
    row=mysql_fetch_row(result);
    if(row==NULL) { bad_row++; break; }
    mysql_field_seek(result,qType);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[qType]) { bad_row++; break; }
    if (row[qType] == NULL) { bad_row++; break; }
    ema = alpha * (strtof(row[qType],NULL) - oldema)  + oldema;
    oldema=ema;
    printf("[%s] =\t%.4f\n",row[6],ema);
    StartRow++;
  }	// end While
  if (bad_row>0) { 
    fprintf(stderr,"2 NULL data found, aborting process\n"); 
    mysql_free_result(result); 
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
