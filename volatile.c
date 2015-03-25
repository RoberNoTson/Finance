// volatile.c
/* measure difference between average highs/lows for 2, 5 and 9 days, sort output
 * 
 * Parms: [Sym] defaults to all
 * compile: gcc -Wall -O2 -ffast-math volatile.c -o volatile `mysql_config --include --libs`
 */

#define		MINVOLUME "80000"
#define		MINPRICE "5"
#define		MAXPRICE "300"

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

int main(int argc, char *argv[]) {
  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  char	*query_main="select distinct(symbol) from stockinfo \
    where exchange in (\"NasdaqNM\",\"NGM\", \"NCM\", \"NYSE\", \"NYQ\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  char query[1024];
  char	query_list[1024];
  char	Sym[12];
  int	x;
  double	low_avg2,high_avg2,low_avg5,high_avg5,low_avg9,high_avg9,diff2,diff5,diff9,day_close,pc2,pc5,pc9;
  unsigned long	*lengths;

  #include	"Includes/print_error.inc"
  #include	"Includes/valid_sym.inc"

  // parse cli parms, if any
  if (argc > 2) {
    printf("Usage:  %s [Sym] (default to all symbols)\nReturns 2 day, 5 day and 9 day averages\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // connect to the database
  #include "Includes/beancounter-conn.inc"
  if (argc == 2) {
    memset(Sym,0,sizeof(Sym));
    for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
    if (valid_sym(Sym)) print_error(mysql,"Aborting process");
    sprintf(query_list,"select distinct(symbol) from stockinfo where symbol = \"%s\"",Sym);
  } else strcpy(query_list,query_main);
  if (mysql_query(mysql,query_list)) print_error(mysql, "Failed to query database");
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 

  //Big Loop through all Symbols
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); break; }
  
    sprintf(query,"select day_low,day_high,day_close from stockprices where symbol = \"%s\" order by date desc limit 9",row_list[0]);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
    low_avg2=low_avg5=low_avg9=high_avg2=high_avg5=high_avg9=day_close=pc2=pc5=pc9=0;
    for (x=0;x<9;x++) {
      row=mysql_fetch_row(result);
      if (row==NULL) {
	puts("Unable to find any valid data");
	mysql_close(mysql);
	exit(EXIT_FAILURE);
      }
      // check for nulls
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { printf("Null data found, results not valid for %s\n",row_list[0]); continue; }
      if (!lengths[1]) { printf("Null data found, results not valid for %s\n",row_list[0]); continue; }
      if (!lengths[2]) { printf("Null data found, results not valid for %s\n",row_list[0]); continue; }
      if (x==0) day_close = strtod(row[2],NULL);
      if (x<2) {
	low_avg2+=strtod(row[0],NULL);
	high_avg2+=strtod(row[1],NULL);
      }
      if (x==2) {
	low_avg2 /= x;
	high_avg2 /= x;
      }
      if (x<5) {
	low_avg5+=strtod(row[0],NULL);
	high_avg5+=strtod(row[1],NULL);
      }
      if (x==5) {
	low_avg5 /= x;
	high_avg5 /= x;
      }
      low_avg9+=strtod(row[0],NULL);
      high_avg9+=strtod(row[1],NULL);
    }	// end For
    mysql_free_result(result);
    low_avg9 /= x;
    high_avg9 /= x;
    diff2 = high_avg2-low_avg2;
    diff5 = high_avg5-low_avg5;
    diff9 = high_avg9-low_avg9;
    pc2 = diff2/day_close*100;
    pc5 = diff5/day_close*100;
    pc9 = diff9/day_close*100;
    printf("%s\t%6.2f\t%.4f\t%.4f\t%.4f\t%.2f%%\t%.2f%%\t%.2f%%\n",row_list[0],day_close,diff2,diff5,diff9,pc2,pc5,pc9);
  }	// end of Big Loop

  // finished with the database
  mysql_free_result(result_list);
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
