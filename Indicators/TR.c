// TR.c
/* The True Range (TR) is designed to measure the volatility between two days.
 * The True Range is defined as the greatest of the following :
 * - The current high less the current low.
 * - The absolute value of : current high less the previous close.
 * - The absolute value of : current low less the previous close.
 * compile:  gcc -Wall -O2 -ffast-math -O2 -o TR TR.c `mysql_config --include --libs`
 */

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char qDate[12];

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"
#include	"../Includes/valid_date.inc"

int main(int argc, char * argv[]) {
  char query[200];
  float A,B,C,TR;
  int Periods=0,x;
  unsigned long	*lengths;
  time_t t;
  struct tm *TM;

  // parse cli parms, if any
  if (argc < 2 || argc>3)  {
    printf("True Range measures the volatility between 2 days\nUsage:  %s Sym {## | yyyy-mm-dd} [Data type]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  valid_sym(argv[1]);
  // use today's date by default
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
  if (argc == 3) {
    if (strlen(argv[2]) == 10) {	  // is parm a date?
      strcpy(qDate, argv[2]);	// yes, process it
    } else {	// number of rows to print
      Periods=atoi(argv[2]);	// not currently used, could be added in the future
    }
  }
  valid_date(argv[1]);
  // query the data
    sprintf(query,"select day_high,day_low,previous_close,date from stockprices where symbol = \"%s\" and date = \"%s\"",argv[1],qDate);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
    row=mysql_fetch_row(result);
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting"); }
    }
    
    // A = Today's High - Today's Low
    A=strtof(row[0],NULL)-strtof(row[1],NULL);
    // B = abs(Prev_Close - Today's High)
    B=fabs(strtof(row[2],NULL) - strtof(row[0],NULL));
    // C = abs(Prev_Close - Today's Low)
    C=fabs(strtof(row[2],NULL) - strtof(row[1],NULL));
    TR=fmax(A,B);
    TR=fmax(TR,C);

  // print the results
  printf("[%s] =\t%.4f\n",row[3],TR);

  // finished with the database
   mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
