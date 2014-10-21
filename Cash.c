// Basic MySQL query program using C interface 
// gcc -Wall -O2 -ffast-math -o Cash Cash.c `mysql_config --include --libs`

#define	QUERY1 "select name, concat('$',sum(value)-sum(cost)) from cash  where date <= NOW() group by name"

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	"Includes/print_error.inc"

static MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;

int	main(int argc, char *argv[]) {
  #include "Includes/beancounter-conn.inc"
  if (mysql_query(mysql,QUERY1)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql)))  print_error(mysql, "Store results or query failed");
  row=mysql_fetch_row(result);
  if ((row==NULL) && (mysql_errno(mysql))) print_error(mysql, "fetch_row failed");
  printf("%s\t%s\n",row[0],row[1]);
  mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}