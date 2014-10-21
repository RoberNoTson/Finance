// ATR.c
/* Average True Index is a measure of volatility. 
 * Parms:  Symbol [Number (def. 14), High, Low, Close]
 * compile:  gcc -Wall -O2 -ffast-math -o ATR ATR.c `mysql_config --include --libs`
 * The Average True Index is a moving average of the True Ranges.
 * The standard ATR works with a fourteen-day parameter : n = 14
 * High ATR values often occur at market bottom following a 'panic' sell-off.
 * Low ATR values are often found during extended sideways periods, 
 * such as those found at tops and after consolidation periods.
 */

#define	MAX_PERIODS 200
#define	_XOPENSOURCE
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

int Usage(char * prog) {
    printf("Usage:  %s Sym [##(def 14)]\n \
    Average True Index is a measure of volatility, a moving average of the True Ranges.\n \
    The standard ATR works with a fourteen-day parameter.\n \
    High ATR values often occur at market bottom following a 'panic' sell-off.\n \
    Low ATR values are often found during extended sideways periods,\n \
    such as those found at tops and after consolidation periods.\n", \
    prog);
    exit(EXIT_FAILURE);
  }
  
int main(int argc, char *argv[]) {
    float ATR=0.0;
    float High,Low,Close,prev_close,A,B,C,TR;
    int	Periods=14;
    int StartRow,num_rows,x,y;
    char query[200];
    unsigned long	*lengths;

    // parse cli parms
  if ((argc < 2) || (argc>3)) Usage(argv[0]);
#include "../Includes/beancounter-conn.inc"
  valid_sym(argv[1]);
  if (argc == 3) {
    Periods=atoi(argv[2]);
  }  
    
  sprintf(query,"select day_high,day_low,day_close,previous_close,date from stockprices where symbol = \"%s\" order by date",argv[1]);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\"\n", argv[1]);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (MAX_PERIODS+1)) {
    printf("Not enough rows found to process SMA for %d periods\n",Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

  // data loaded, start processing
  //  StartRow=num_rows-MAX_PERIODS-Periods;
  StartRow=num_rows-MAX_PERIODS;
  while (StartRow < (num_rows-Periods+1)) {
    mysql_data_seek(result, StartRow);
    ATR=0.0;
    for (x=StartRow;x<StartRow+Periods;x++) {
      row=mysql_fetch_row(result);
    // check for nulls
      if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting"); }
      lengths=mysql_fetch_lengths(result);
      for (y=0;y<mysql_num_fields(result);y++) {
	if (!lengths[y]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting"); }
      }      
      High=strtof(row[0],NULL);
      Low=strtof(row[1],NULL);
      Close=strtof(row[2],NULL);
      prev_close=strtof(row[3],NULL);
      A=High-Low;
      B=fabs(prev_close - High);
      C=fabs(prev_close - Low);
      TR=fmax(A,B);
      TR=fmax(TR,C);
      ATR += TR;
    }
   ATR /= Periods;
   printf("[%s] =\t%.4f\n",row[4],ATR);
   StartRow++;
  }	// end While
 
  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
