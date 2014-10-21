// SMA.c 
/* A simple arithmetic moving average, e.g. (A+B+C)/3
 * Parms:  Symbol Number|Date [Data-type]
 * The first argument is the period used to calculed the average.
 * The second argument is optional; default is Close prices.
 * Other available values are: High, Low, Open, Change, Volume
 * compile:  gcc -Wall -O2 -ffast-math -o SMA SMA.c `mysql_config --include --libs`
 */

#define		MAX_PERIODS 200
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <time.h>
#include	<ctype.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char	qDate[12];

#include        "../Includes/print_error.inc"
#include        "../Includes/valid_sym.inc"
#include        "../Includes/valid_date.inc"


int main(int argc, char *argv[]) {
  int	Periods=50;
  int qType=3;
  int	bad_row=0;
  int num_rows,x,StartRow;
  char query[200];
  char	Sym[32];
  float sum=0.0,sma=0.0;
  MYSQL_FIELD *field;
  time_t t;
  struct tm *TM;
  unsigned long	*lengths;

  // parse cli parms
  if (argc == 1) {
    printf("Usage:  %s Sym [periods(50) | yyyy-mm-dd]] [close(default)|open|high|low|volume|change]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  if (argc >= 2) {
    for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
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
    if (strlen(argv[2]) == 10) {	// yes, process it
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
      printf("Usage:  %s Sym [periods(50) | yyyy-mm-dd]] [close(default)|open|high|low|volume|change]\n", argv[0]);
  #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
  }
   if (argc > 4) {
    printf("Usage:  %s Sym [Periods(50) | yyyy-mm-dd]] [close(default)|open|high|low|volume|change]\n", argv[0]);
    exit(EXIT_FAILURE);
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
    printf("Not enough rows found to process SMA for %d periods\n",Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  
// calculate the requested SMA  
  StartRow=num_rows-MAX_PERIODS-Periods;
  while (StartRow < (num_rows-Periods+1)) {
    mysql_data_seek(result, StartRow);
    sum=0.0;
    for (x=StartRow;x<StartRow+Periods;x++) {
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
      fprintf(stderr,"NULL data found, aborting process\n"); 
      mysql_free_result(result); 
      #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
    sma = sum/Periods;
    printf("[%s] =\t%.4f\n",row[6],sma);
    StartRow++;
  }
  mysql_free_result(result);
  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);

}
