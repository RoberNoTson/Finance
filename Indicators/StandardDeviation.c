// StandardDeviation.c
/* Standard Deviation is a statistical measure of volatility. 
 * Standard Deviation is typically used as a component of other indicators, 
 * rather than as a stand-alone indicator. 
 * For example, Bollinger Bands are calculated by adding a security's Standard Deviation to a moving average.
 * High Standard Deviation values occur when the data item being analyzed (e.g., prices or an indicator)
 * is changing dramatically. Similarly, low Standard Deviation values occur when prices are stable.
 * Many analysts feel that major tops are accompanied with high volatility 
 * as investors struggle with both euphoria and fear. 
 * Major bottoms are expected to be calmer as investors have few expectations of profits.
 * 
 * Parms:  Sym [Periods(20)] [close(default)|open|high|low|volume|change]
 * compile:  gcc -Wall -O2 -ffast-math StandardDeviation.c -o StandardDeviation `mysql_config --include --libs`
 */

#define		MAX_PERIODS	200
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<errno.h>

MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];

#include        "../Includes/print_error.inc"
#include	"../Includes/valid_date.inc"
#include	"../Includes/valid_sym.inc"

int	main(int argc, char *argv[]) {
  time_t t;
  struct tm *TM;
  unsigned long	*lengths;
  char	query[200];
  int	Periods=20;
  int	StartRow,num_rows,x,y;
  int	qType=0;
  float	sum,sd,sd_sum=0,sma=0;
  float	*sma_list;
  
  if (argc == 1 || argc>4) {
    printf("Usage:  %s Sym [Periods(20)] [close(default)|open|high|low|volume|change]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
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
  
 // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) if(valid_sym(argv[1])) print_error(mysql,"Exiting");
  if (argc >= 3) Periods=atoi(argv[2]);
  if (argc == 4) {
    if (strcasecmp(argv[3], "close") == 0) qType=0;
    else if (strcasecmp(argv[3], "open") == 0) qType=1;
    else if (strcasecmp(argv[3], "high") == 0) qType=2;
    else if (strcasecmp(argv[3], "low") == 0) qType=3;
    else if (strcasecmp(argv[3], "change") == 0) qType=4;
    else if (strcasecmp(argv[3], "volume") == 0) qType=5;
    else {
      printf("Usage:  %s Sym [Periods(20)] [close(default)|open|high|low|change|volume]\n", argv[0]);
  #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
  }
  valid_date(argv[1]);
  
  // query database for required values
  sprintf(query,"select day_close,day_open,day_high,day_low,day_change,volume,date from stockprices where symbol = \"%s\" and date <= \"%s\" order by date",argv[1],qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol %s\n",argv[1]);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (MAX_PERIODS+Periods)) {
    printf("Not enough rows found to process Standard Deviation for %d periods\n",Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

  // good data retrieved, begin processing
  // Get the SMA value
  if ((sma_list=calloc(num_rows, sizeof(float)))==NULL) {
    mysql_free_result(result); 
    print_error(mysql,"calloc failed, aborting!"); 
  }
  
  StartRow=num_rows-MAX_PERIODS;
  y=0;
  while (StartRow < (num_rows-Periods+1)) {
    mysql_data_seek(result, StartRow);
    sum=0.0;
    
    for (x=StartRow;x<StartRow+Periods;x++) {	// loop Periods(20) times
      row=mysql_fetch_row(result);
    // error check for nulls
      if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[qType]) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
      sum += strtof(row[qType], NULL);
    }
    sma = sum/Periods;
    sma_list[y]=sma;
    y++;
    StartRow++;
  }

  StartRow=num_rows-MAX_PERIODS;
  y=0;
  while (StartRow < (num_rows-Periods+1)) {
    mysql_data_seek(result, StartRow);
    sd_sum=0.0;
    for (x=StartRow;x<StartRow+Periods;x++) {	// loop Periods(20) times
      row=mysql_fetch_row(result);
    // error check for nulls
      if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
      lengths=mysql_fetch_lengths(result);
      if (!lengths[qType]) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
    sd_sum += pow((strtof(row[qType],NULL)-sma_list[y]),2);
    }
    
    sd = sqrtf(sd_sum/Periods);
    printf("[%s] =\t%.4f\n",row[6],sd);
    y++;
    StartRow++;
  }  
  
  free(sma_list);
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
