/* cash_activity.c
 * replacement for cach_activity.pl
 * 
 * List activity and running totals from beancounter.cash
 * compile:  gcc -Wall -O2 -ffast-math -o cash_activity cash_activity.c `mysql_config --include --libs`
 */

// set DEBUG to 1 for testing, 0 for production
#define         DEBUG   0
#define         _XOPEN_SOURCE
#define         _XOPENSOURCE
//#define	QUERY1 "select value, cost from cash where value is not null order by date, name"
#define	QUERY1 "select date, value, cost from cash where value is not null order by date, name"

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include	<string.h>
#include	<monetary.h>
//#include	<math.h>

int	atocents(char *val) { return(lrintf(strtof(val,NULL) * 100)); }

MYSQL *mysql;
#include        "Includes/print_error.inc"

int main(int argc, char * argv[]) {
  int	value=0;
  int	cost=0;
  int	cash=0;
  char	s_cash[32];
  MYSQL_RES *result;
  MYSQL_ROW row;

  #include "Includes/beancounter-conn.inc"
  if (mysql_query(mysql,QUERY1)) print_error(mysql, "Failed to query database");
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed");
  while ((row=mysql_fetch_row(result)) != NULL) {
    value = atocents(row[1]);
    cost = atocents(row[2]);
    cash += (value - cost);
    strfmon(s_cash,sizeof(s_cash),"%.2n",(float)cash/100);
    if (DEBUG) printf("cash: %d\tvalue: %d\tcost: %d\n",cash,value,cost);
    printf("%-10s  %12s %12.2f\t%6.2f\n",row[0],s_cash,(float)value/100,(float)cost/100);
  }
  // finished with the database
  mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
