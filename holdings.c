// holdings.c
/* display info about current holdings
 * Parms: none
 * compile: gcc -Wall -O2 -ffast-math holdings.c -o holdings `mysql_config --include --libs`
 */

#define	DEBUG	0
#define		_XOPENSOURCE
#define	CASH_QUERY "select name, sum(value)-sum(cost) from beancounter.cash where date <= NOW() group by name"

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<errno.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"

int	main (int argc, char *argv[]) {
  char	query[1024];
  MYSQL_RES *result;
  MYSQL_ROW row;
  int	x,num_rows;
  float	total=0;
  
  #include "Includes/beancounter-conn.inc"
  // select all holdings for menu
  sprintf(query,"select p.symbol, a.buy_date, a.buy_price, p.current_shares, a.buy_shares, a.ID, \
    (select sum(buy_shares)-sum(sell_shares) from Investments.activity where symbol=p.symbol \
    and Lot_id=a.ID group by Lot_id) as lot_size \
    from Investments.portfolio p, Investments.activity a \
    where p.current_shares > 0 \
    and a.symbol=p.symbol \
    and a.buy_shares > a.sell_shares \
    order by symbol, buy_date, current_shares");
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) print_error(mysql,"Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  for (x=0;x<66;x++) printf("=");
  puts("\n\t\t\tCurrent Holdings");
  for (x=0;x<66;x++) printf("-");
  puts("\nSymbol\t    Cost\tShares\t\t    Stop\t  Pur Date");
  for (x=0;x<66;x++) printf("-");
  printf("\n");
  // create listing of holdings
  for (x=1;x<=num_rows;x++) {
    row=mysql_fetch_row(result);
    if(row==NULL) break;
    if (atoi(row[6])==0) continue;
    printf("%s\t%8.2f\t%s [of %s]\t%8.2f\t%s\n",row[0],strtof(row[2],NULL),row[6],row[3],strtof(row[2],NULL)-(strtof(row[2],NULL)*0.06),row[1]);
    total += strtof(row[2],NULL) * strtof(row[4],NULL);
  }
  if (!num_rows) puts("(none)");
  
  // query cash on hand
  if (mysql_query(mysql,CASH_QUERY)) {
    print_error(mysql, "Failed to query database");
    return(1);
  }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) {	// no results returned, might be okay if query was not a 'select'
      print_error(mysql, "Store results or query failed");
      return(1);
  } 
  row=mysql_fetch_row(result);
  if ((row==NULL) && (mysql_errno(mysql))) {	// no results returned and an error was reported
      print_error(mysql, "fetch_row failed");
      return(1);
  }
  // show cash on hand
  for (x=0;x<66;x++) printf("-");
//  printf("\n%s\t$%s\n",row[0],row[1]);
  printf("\nCash\t$%s\n",row[1]);
  for (x=0;x<66;x++) printf("-");
  total += strtof(row[1],NULL);
  printf("\nTotal\t$%8.2f\n",total);
  for (x=0;x<66;x++) printf("=");
  printf("\n");
  
  // finished with the database
  mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
