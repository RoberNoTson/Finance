// BOL.c -- Bollinger band calc
/* Bollinger Bands are similar to moving average envelopes. 
 * The difference between Bollinger Bands and envelopes is envelopes are plotted at 
 * a fixed percentage above and below a moving average (SMA), 
 * whereas Bollinger Bands are plotted at Standard Deviation levels above and below a moving average.
 * The standard Bollinger Bands (BOL 20(Periods)-2(coefficient))
 * 
 * Parms: Sym [period [nsd]] [close(default)|open|high|low]
 * compile: gcc -Wall -O2 -ffast-math BOL.c -o BOL `mysql_config --include --libs`
 */

#define		MAX_PERIODS	200
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

int Usage(char * prog) {
    printf("Usage:  %s Sym [Periods(20)] [coeff(2)] [close(default)|open|high|low]\n \
    Bollinger Bands are similar to moving average envelopes.\n \
    The difference between Bollinger Bands and envelopes is envelopes are plotted at\n \
    a fixed percentage above and below a moving average (SMA),\n \
    whereas Bollinger Bands are plotted at Standard Deviation levels\n \
    above and below a moving average.\n",
    prog);
    exit(EXIT_FAILURE);
  }
  
int	main(int argc, char *argv[]) {
  unsigned long	*lengths;
  char	query[200];
  char	Sym[12];
  int	Periods=20;
  int	StartRow,num_rows,x,y;
  int qType=3;
  float	coeff=2.0;
  float	sum,sd,sd_sum=0,sma=0;
  float	bolsup,bolinf;
  float	*sma_list;

  if (argc == 1 || argc>6) Usage(argv[0]);
 // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) {
    memset(Sym,0,sizeof(Sym));
    for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
    if (valid_sym(Sym)) print_error(mysql,"Exiting");
  }
  if (argc >= 3) {
    if (isdigit((int) *argv[2])) Periods=strtof(argv[2],NULL);
    else if (strcasecmp(argv[2], "open") == 0) qType=0;
    else if (strcasecmp(argv[2], "low") == 0) qType=1;
    else if (strcasecmp(argv[2], "high") == 0) qType=2;
    else if (strcasecmp(argv[2], "close") == 0) qType=3;
    else Usage(argv[0]);
  }
  if (argc >= 4) {
    if (isdigit((int) *argv[3])) coeff=strtof(argv[3],NULL);
    else if (strcasecmp(argv[3], "open") == 0) qType=0;
    else if (strcasecmp(argv[3], "low") == 0) qType=1;
    else if (strcasecmp(argv[3], "high") == 0) qType=2;
    else if (strcasecmp(argv[3], "close") == 0) qType=3;
    else Usage(argv[0]);
  }
  if (argc == 5) {
    if (strcasecmp(argv[4], "open") == 0) qType=0;
    else if (strcasecmp(argv[4], "low") == 0) qType=1;
    else if (strcasecmp(argv[4], "high") == 0) qType=2;
    else if (strcasecmp(argv[4], "close") == 0) qType=3;
    else Usage(argv[0]);
  }

  // query database for required values
  sprintf(query,"select day_open,day_low,day_high,day_close,date from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
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
    // Get 20 period SMA value
  if ((sma_list=calloc(num_rows, sizeof(float)))==NULL) {
    mysql_free_result(result); print_error(mysql,"calloc failed, aborting!"); 
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
  }	// end While

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

    // Bollinger Band Sup is equal to the value of the moving average + standard deviation
    bolsup = sma_list[y] + (coeff * sd);
    // Bollinger Band Inf is equal to the value of the moving average - standard deviation
    bolinf = sma_list[y] - (coeff * sd);
    printf("[%s] =\t%.4f\t%.4f\t%.4f\n",row[1],sma_list[y],bolsup,bolinf);
    y++;
    StartRow++;
  }	// end While
    
  free(sma_list);
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
