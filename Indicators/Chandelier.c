/* Chandelier.c
 * Parms:  [Periods (22), Coefficient (3)]
 * The Chandelier Exit provides Stops for closing Long or Short positions
 *   ChanUp=Max-(Coeff*ATR);
 *   ChanDn=Min+(Coeff*ATR);
 * 
 * compile:  gcc -Wall -ffast-math -O2 -o Chandelier Chandelier.c `mysql_config --include --libs`
 */

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

static MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char qDate[12];

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"
#include	"../Includes/valid_date.inc"

int main(int argc, char * argv[]) {
  char query[200];
  char Sym[16];
  float Max,Min,trow;
  float	ATR=0;
  float PrevHigh,PrevLow,PrevClose,PrevPrevClose,A,B,C,TR;
  int	Coeff=3;
  int	ATR_Periods=22;
  int	MinMax_Periods=22;
  int	StartRow;
  int	x;
  time_t t;
  struct tm *TM;
  
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  
  // parse cli parms, if any
  if (argc == 1) {
    printf("Usage:  %s Sym {[yyyy-mm-dd] | [##]}\n", argv[0]);
    exit(EXIT_FAILURE);
  }
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
  if (argc == 3) {
    // is it a date?
    if (strlen(argv[2]) == 10) {	// yes, process it
      strcpy(qDate, argv[2]);
    } else {  // number of rows to print
      ATR_Periods=atoi(argv[2]);
    }
  }  
  valid_date(Sym);

  // get MaxInPeriod of PrevHighs
  sprintf(query,"select day_high from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  StartRow=mysql_num_rows(result)-MinMax_Periods;
  Max=0.0;
  mysql_data_seek(result, StartRow);
  while ((row=mysql_fetch_row(result))) {
    trow = strtof(row[0],NULL);
    if (trow>Max) Max=trow;
  }
  mysql_free_result(result);

  // get MinInPeriod of PrevLows
  sprintf(query,"select day_low from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  StartRow=mysql_num_rows(result)-MinMax_Periods;
  Min=FLT_MAX;
  mysql_data_seek(result, StartRow);
  while ((row=mysql_fetch_row(result))) {
    trow = strtof(row[0],NULL);
    if (trow<Min) Min=trow;
  }
  mysql_free_result(result);

  // get ATR
  sprintf(query,"select day_high,day_low,day_close,previous_close from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  StartRow=mysql_num_rows(result)-ATR_Periods;
  mysql_data_seek(result, StartRow);
  while ((row=mysql_fetch_row(result))) {
    PrevHigh=strtof(row[0],NULL);
    PrevLow=strtof(row[1],NULL);
    PrevClose=strtof(row[2],NULL);
    PrevPrevClose=strtof(row[3],NULL);
    A=PrevHigh-PrevLow;
    B=fabs(PrevPrevClose - PrevHigh);
    C=fabs(PrevPrevClose - PrevLow);
    TR=fmax(A,B);
    TR=fmax(TR,C);
    ATR += TR;
  }
  mysql_free_result(result);
  ATR /= ATR_Periods;

  // calculate and print ChadUp/Down 
  printf("[%s] =\t%.4f\t%.4f\n",qDate, Max-(Coeff*ATR), Min+(Coeff*ATR));
  
  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
