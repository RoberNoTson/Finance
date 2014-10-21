// *** IN WORK *** STO.c
/* The Stochastic Oscillator
 * a momentum indicator that shows the location of the current close
 * relative to the high/low range over a set number of periods. 
 * Closing levels that are consistently near the top of the range indicate
 * accumulation (buying pressure) and those near the bottom of the range
 * indicate distribution (selling pressure).
 * 
 * %K Fast = 100 * ((Last - Lowest Low(n)) / (Highest High(n) - Lowest Low(n)))
 * %D Fast = M-periods SMA of %K Fast
 * %K Slow = A-periods SMA of %K Fast
 * %D Slow = B-periods SMA of %K Slow
 * %K Fast corresponds to STO/1, %D Fast to STO/2, %K Slow to STO/3 and %D Slow to STO/4
 * %K Slow may also be known as the stochastic oscillator
 * %D Slow is also known as the signal line
 * 
 * Parms: Sym [Periods(5) [Mperiods(3) Aperiods(3) Bperiods(3)] ]
 * 
 * The first argument is the Period n used in the formula above and for each of the subsequent SMA periods.
 * Shorter periods (5-14 days) will generate more overbought/oversold signals, than longer perions (15-40 days)
 * The second argument is M-periods, the third is A-periods, fourth is B-periods. 
 * These are almost always left at the default 3 days for smoothing
 * 
 * compile: gcc -Wall -O2 -ffast-math STO.c -o STO `mysql_config --include --libs`
 */

#define		MAX_PERIODS 200
#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>

static MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"

int main(int argc, char * argv[]) {
  char	query[200];
  float	tval=0;
  float	k_fast=0,k_slow=0,d_fast=0,d_slow=0;
  float	sum=0.0;
  float	LowestLow=FLT_MAX;
  float	HighestHigh=0.0;
  float	*k_fast_hist,*k_slow_hist;
  int	Periods=5,Mperiods=3,Aperiods=3,Bperiods=3;
  int	x,y,z,num_rows,StartRow;
  unsigned long *lengths;

  // parse cli parms
  if (argc==1 || argc>6) {
    printf("Usage:  %s Sym [Periods(5) [Mperiods(3) Aperiods(3) Bperiods(3)] ]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc >= 3) Periods=atoi(argv[2]);
  if (argc >= 4) Mperiods=atoi(argv[3]);
  if (argc >= 5) Aperiods=atoi(argv[4]);
  if (argc == 6) Bperiods=atoi(argv[5]);
  

  // connect to the database
  #include "../Includes/beancounter-conn.inc"
  valid_sym(argv[1]);
  sprintf(query,"select day_close, day_high, day_low, date from stockprices where symbol = \"%s\" order by date",argv[1]);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\"\n", argv[1]);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (MAX_PERIODS+Periods)) {
    printf("Not enough rows found to process STO for %d periods\n",Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  // allocate memory chunks for values
  if ((k_fast_hist=calloc(Mperiods, sizeof(float)))==NULL) {
    mysql_free_result(result); print_error(mysql,"M calloc failed, aborting!"); 
  }
  if ((k_slow_hist=calloc(Bperiods, sizeof(float)))==NULL) {
    mysql_free_result(result); print_error(mysql,"B calloc failed, aborting!"); 
  }
  
  // calculate the requested STO
  StartRow=num_rows-MAX_PERIODS-Periods;
  z=0;
  while (StartRow < (num_rows-Periods+1)) {
    mysql_data_seek(result, StartRow);
  // find Highest High, Lowest Low
    LowestLow=FLT_MAX;
    HighestHigh=0.0;
    for (x=0; x<Periods; x++) {
      row=mysql_fetch_row(result);
      if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); break; }
      lengths=mysql_fetch_lengths(result);
      for (y=0;y<mysql_num_fields(result);y++) {
	if (!lengths[y]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
      }
      
      tval = strtof(row[1],NULL);
      HighestHigh=fmax(tval,HighestHigh);
      tval = strtof(row[2],NULL);
      LowestLow=fmin(LowestLow,tval);
    }
      k_fast = 100 * ((strtof(row[0],NULL) - LowestLow) / (HighestHigh - LowestLow));

      // calculate SMA for M, A, B
      if (z==0)
	for (y=0; y<Mperiods; y++) k_fast_hist[x] = k_fast;
      else {
	sum=0;
	for (y=0; y<Mperiods; y++) {
	  k_fast_hist[y] = y<Mperiods-1 ? k_fast_hist[y+1] : k_fast;
	  sum += k_fast_hist[y];
	}
	d_fast = sum/Mperiods;
	k_slow = sum/Aperiods;
      }
      
      if (z==0)
	for (y=0; y<Bperiods; y++) k_slow_hist[x] = k_slow;
      else {
	sum=0;
	for (y=0; y<Bperiods; y++) {
	  k_slow_hist[y] = y<Bperiods-1 ? k_slow_hist[y+1] : k_slow;
	  sum += k_slow_hist[y];
	}
	d_slow = sum/Bperiods;
      }
    
    printf("[%s] =\t%.4f\t%.4f\t%.4f\t%.4f\n",row[3],k_fast,d_fast,k_slow,d_slow);
    StartRow++;
    z++;
  } // end While

  mysql_free_result(result);
  free(k_fast_hist);
  free(k_slow_hist);

  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
