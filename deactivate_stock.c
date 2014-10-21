// deactivate_stock.c
/* update beancounter.stockinfo to set the ACTIVE flag to FALSE
 * Parms: Sym
 * compile: gcc -Wall -O2 -ffast-math -o deactivate_stock deactivate_stock.c `mysql_config --include --libs`
 */

#define		DEBUG	0
#define		_XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>
#include	<ctype.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"
#include        "Includes/valid_sym.inc"

int	main(int argc, char *argv[]) {
  char	query[1024];
  int	x;
  char	Sym[32];
  MYSQL_RES *result;
  
  // parse cli parms
  if (argc != 2) {
    printf("Usage:  %s Sym\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  // convert symbol parm to uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  // verify symbol exists in stockinfo, else exit
  #include "Includes/beancounter-conn.inc"
  valid_sym(Sym);
  sprintf(query,"select active from stockinfo where symbol = \"%s\" and active = true",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Fatal error - failed to query database in deactivate_stock");
  result=mysql_store_result(mysql);
  if (mysql_num_rows(result) == 0) print_error(mysql,"Already deactivated, no action taken\n");
  // check for current holdings
  sprintf(query,"select * from portfolio where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Fatal error - failed to query database in deactivate_stock");
  result=mysql_store_result(mysql);
  if (mysql_num_rows(result)) print_error(mysql,"Current holdings found for this symbol, no action taken");
  
  sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "02 Fatal error - failed to update database in deactivate_stock");
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  printf("%s deactivated\n",Sym);
  exit(EXIT_SUCCESS);
}
