// MACD.c - Moving Average Convergence/Divergence
/* use CLOSE prices to calculate the values for
 * MACD(12 day EMA/close), MACDsignal(26 day EMA/close), MACDdifference(9 day EMA of the EMA line)
 * 
 * Parms:  Sym [ Period1(12) Period2(26) Period3(9) ] [End-date]
 * compile: gcc -Wall -O2 -ffast-math -o MACD MACD.c `mysql_config --include --libs`
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
//#include	"../Includes/valid_date.inc"
#include	"../Includes/valid_sym.inc"

int Usage(char *prog) {
    printf("Usage:  %s Sym [ Period1(12) Period2(26) Period3(9) ] [End-date] [close|open|high(default)|low|volume|change]\n", prog);
    exit(EXIT_FAILURE);
  }

int	main(int argc, char *argv[]) {
  float	first_ema=0,second_ema=0,third_ema=0,macd=0,alpha1,alpha2,alpha3,tval;
  float	Period1=12,Period2=26,Period3=9;
  float	oldema1=0.0,oldema2=0.0,oldema3=0.0;
  time_t t;
  struct tm *TM;
  int	x,y;
  unsigned long	*lengths;
  char	query[200];
  float	*third_ema_list=NULL;
  int	Max_Periods=MAX_PERIODS;
  int	StartRow,num_rows;
  int	qType=3;
  
  // cli parms
  if (argc == 1 || argc>7) Usage(argv[0]);
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
  if (argc >= 2) if (valid_sym(argv[1])) print_error(mysql,"Exiting");
  if (argc >= 3) {
    // is it a date?
    if (strlen(argv[2]) == 10) {	// yes, process it
      strcpy(qDate, argv[2]);
    } else { 
	Period1=atoi(argv[2]);
    }
  }
  if (argc >= 4) {
    // is it a date?
    if (strlen(argv[3]) == 10) {	// yes, process it
      strcpy(qDate, argv[3]);
    } else {  
	Period2=atoi(argv[3]);
    }
  }
  if (argc >= 5) {
    // is it a date?
    if (strlen(argv[4]) == 10) {	// yes, process it
      strcpy(qDate, argv[4]);
    } else {  
	Period3=atoi(argv[4]);
    }
  }
  if (argc == 6) {
    // is it a date?
    if (strlen(argv[5]) == 10) {	// yes, process it
      strcpy(qDate, argv[5]);
    } else {  // invalid entry
      Usage(argv[0]);
    }
  }
  if (argc == 7) {
    if (strcasecmp(argv[6], "open") == 0) qType=0;
    else if (strcasecmp(argv[6], "low") == 0) qType=1;
    else if (strcasecmp(argv[6], "high") == 0) qType=2;
    else if (strcasecmp(argv[6], "close") == 0) qType=3;
    else if (strcasecmp(argv[6], "change") == 0) qType=4;
    else if (strcasecmp(argv[6], "volume") == 0) qType=5;
    else Usage(argv[0]);
  }
//  valid_date(argv[1]);
  
  // query database for required values
  // query all data
  sprintf(query,"select day_open,day_low,day_high,day_close,day_change,volume,date from stockprices where symbol = \"%s\" and date <= \"%s\" order by date",argv[1],qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol %s\n",argv[1]);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  
  // good data retrieved, begin processing
  alpha1 = 2 / (float)(Period1 + 1);
  alpha2 = 2 / (float)(Period2 + 1);
  alpha3 = 2 / (float)(Period3 + 1);
  Max_Periods = num_rows > Max_Periods ? Max_Periods : num_rows;
  StartRow = num_rows-Max_Periods;
  if ((third_ema_list=calloc(Max_Periods, sizeof(float)))==NULL) {	// alloc mem for 3rd EMA table
    mysql_free_result(result); print_error(mysql,"calloc failed, aborting!"); 
  }

  // Get the EMA values and calculate and stores the MACD values
  //  first_ema 
  mysql_data_seek(result,(StartRow));
  row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"1 NULL data found, aborting!"); }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[qType]) { mysql_free_result(result); print_error(mysql,"2 NULL data found, aborting!"); }
  // get the starting oldema value  
  oldema1 = oldema2 = oldema3 = strtof(row[qType],NULL);
  // continue on with next row after getting initial oldema
  y=1;
  for(x=StartRow+1; x<num_rows; x++) {
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"4 NULL data found, aborting!"); }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[qType]) { mysql_free_result(result); print_error(mysql,"5 NULL data found, aborting!"); }
    if (!lengths[6]) { mysql_free_result(result); print_error(mysql,"5 NULL data found, aborting!"); }
    tval = strtof(row[qType],NULL);
    first_ema = (tval - oldema1) * alpha1 + oldema1;
    second_ema = (tval - oldema2) * alpha2 + oldema2;
    oldema1=first_ema;
    oldema2=second_ema;
    macd = first_ema - second_ema;
    oldema3 = oldema1 - oldema2;
    third_ema = (oldema3 - third_ema) * alpha3 + third_ema;
    oldema3 = third_ema;
    printf("[%s] =\t%.4f\t%.4f\t%.4f\n",row[6],macd,third_ema,macd - third_ema);
    y++;
  }	// end of for loop
  
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  free(third_ema_list);
  exit(EXIT_SUCCESS);
}
