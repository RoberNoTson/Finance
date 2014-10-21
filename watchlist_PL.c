// watchlist_PL.c
/* P/L report for watchlist stocks
 * 
 * Parms: [date] default to most current
 * compile: gcc -Wall -O2 -ffast-math watchlist_PL.c -o watchlist_PL `mysql_config --include --libs`
 */

#define	DEBUG	0
#define _XOPENSOURCE
#define		MAX_PERIODS	200
#define		MINVOLUME "80000"
#define		MINPRICE "14"
#define		MAXPRICE "250"

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>
#include	<error.h>
#include	<sys/types.h>
#include	<sys/wait.h>

MYSQL *mysql;
#include	"Includes/print_error.inc"

int main (int argc, char *argv[]) {
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char	qDate[12];
  char	priorDate[12];
  char	prevpriorDate[12];
  char	query[1024];
  unsigned long *lengths;
  FILE	*out_file;
  char	*out_filename="/Finance/Investments/watchlist_summary.txt";
  char	*perms="w+";
    
  if (argc > 2 || (argc == 2 && !strcmp(argv[1],"-h"))) {
    printf("Usage: %s [date] [-h])\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc == 1)
    sprintf(query,"select distinct(date) from stockprices where date <= NOW() order by date desc limit 3");
  if (argc == 2) 
    sprintf(query,"select distinct(date) from stockprices where date <= \"%s\" order by date desc limit 3",argv[1]);

  #include "Includes/beancounter-conn.inc"
  mysql_query(mysql,query);
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql,"No data found in stockprices");
  if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql,"fetch row failed");
  if ((lengths = (mysql_fetch_lengths(result))) == NULL) print_error(mysql,"Error determing lengths");
  if (!lengths[0])  print_error(mysql,"Null current date found");
  strcpy(qDate,row[0]);
  if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql,"fetch row failed");
  if ((lengths = (mysql_fetch_lengths(result))) == NULL) print_error(mysql,"Error determing lengths");
  if (!lengths[0])  print_error(mysql,"Null current date found");
  strcpy(priorDate,row[0]);
  if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql,"fetch row failed");
  if ((lengths = (mysql_fetch_lengths(result))) == NULL) print_error(mysql,"Error determing lengths");
  if (!lengths[0])  print_error(mysql,"Null current date found");
  strcpy(prevpriorDate,row[0]);
  if (DEBUG) printf("qDate: %s\tpriorDate: %s\tprevpriorDate: %s\n",qDate,priorDate,prevpriorDate);
  mysql_free_result(result);

  // print daily P/L report for paper trades
  sprintf(query,"select concat(if(OB=true,'OB',''), \
      case when (KR=true && OB=true) then ',KR' \
	when (KR=true && OB=false) then 'KR' else '' end, \
      case when (TREND=true && (KR=true || OB=true)) then ',Trend' \
	when (TREND=true && KR=false && OB=false) then 'Trend' else '' end, \
      case when (MACD=true && (KR=true || OB=true || TREND=true)) then ',MACD' \
	when (MACD=true && KR=false && OB=false && TREND=false) then 'MACD' else '' end, \
      case when (HV=true && (TREND=true || MACD=true || KR=true || OB=true)) then ',HV' \
	when (HV=true && TREND=false && MACD=false && KR=false && OB=false) then 'HV' else '' end), \
      symbol, PAPER_BUY_PRICE, PAPER_SELL_PRICE, PAPER_PL from Investments.watchlist \
      where date = \"%s\" and PAPER_PL is not null order by PAPER_PL",prevpriorDate);
  if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query watchlist"); }
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql,"No paper P/L data found");
  out_file=fopen(out_filename,perms);
  printf("Paper Trade Results for stocks bought %s, sold %s\n",priorDate,qDate);
  fprintf(out_file,"Paper Trade Results for stocks bought %s, sold %s\n",priorDate,qDate);
  printf("%20s\tSym\tCost\tSell\tP/L\n"," ");
  fprintf(out_file,"%20s\tSym\tCost\tSell\tP/L\n"," ");
  while ((row=mysql_fetch_row(result)) != NULL) {
    printf("%20s\t%s\t%.2f\t%.2f\t%.2f\n",row[0],row[1],strtof(row[2],NULL),strtof(row[3],NULL),strtof(row[4],NULL));
    fprintf(out_file,"%20s\t%s\t%.2f\t%.2f\t%.2f\n",row[0],row[1],strtof(row[2],NULL),strtof(row[3],NULL),strtof(row[4],NULL));
  }
  sprintf(query,"select sum(PAPER_PL), avg(PAPER_PL) from Investments.watchlist where PAPER_PL is not null and date = \"%s\"",prevpriorDate);
  mysql_query(mysql,query);
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql,"no PAPER_PL found");
  row=mysql_fetch_row(result);
  printf("Total P/L today: $%s\tAverage P/L per stock: $%s\n",row[0],row[1]);
  fprintf(out_file,"Total P/L today: $%s\tAverage P/L per stock: $%s\n",row[0],row[1]);
  mysql_free_result(result);
  fclose(out_file);
  
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
