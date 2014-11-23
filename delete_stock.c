/* delete_stock.c
 * Parms: Sym
 * 
 * delete a stock from databases, completely removing all history and info
 * 1. if stock is not active, will ask to deactivate it rather than delete
 * 2. if stock has price history or portfolio history, will ask 2 times for confirmation
 * Databases:  
 * compile:  gcc -Wall -O2 -ffast-math delete_stock.c -o delete_stock `mysql_config --include --libs`
 */

#define         DEBUG   0
#define         _XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <unistd.h>
#include        <ctype.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"

int main(int argc, char * argv[]) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  char  query[1024];
  char  Sym[32];
  int	x,y;
  
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
  sprintf(query,"select symbol,active from stockinfo where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  if (!mysql_num_rows(result)) print_error(mysql,"Symbol not found in stockinfo, no action taken");
  if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql,"fetch row failed");
  if (strcmp(row[1],"0")) print_error(mysql,"Symbol is still active, please deactivate it first");
  sprintf(query,"select symbol from portfolio where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "02 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  if (mysql_num_rows(result)) print_error(mysql,"Active holdings found in this stock, no delete possible");

  // passed all qualifiers, confirm deletion
  printf("Delete %s permanently from all database tables? [y/N]: ",Sym);
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  if (getchar() != '\n') {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
    
  // get number of rows to be deleted and re-confirm
  sprintf(query,"select * from stockprices where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  if ((x = mysql_num_rows(result)) == 0) print_error(mysql,"Symbol not found in stockinfo, no action taken");
  printf("Please confirm you want to delete %d rows from stockprices",x);
  sprintf(query,"select * from Investments.activity where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "02 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  if ((y = mysql_num_rows(result))) printf(" and %d rows from activity? [y/N]",y);
    else printf("? [y/N]: ");
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  if (getchar() != '\n') {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  mysql_free_result(result);

  // delete the stock
  sprintf(query,"delete from stockinfo where symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from stockinfo database");
  if (DEBUG) printf("%s\n",query);
  sprintf(query,"delete from stockprices where symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from stockprices database");
  if (DEBUG) printf("%s\n",query);
  sprintf(query,"delete from Investments.activity where symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from Investments.activity database");
  if (DEBUG) printf("%s\n",query);
  sprintf(query,"delete from Investments.Dividends where symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from Investments.Dividends database");
  if (DEBUG) printf("%s\n",query);
  sprintf(query,"delete from Investments.portfolio where symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from Investments.portfolio database");
  if (DEBUG) printf("%s\n",query);
  sprintf(query,"delete from Investments.watchlist where symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from Investments.watchlist database");
  if (DEBUG) printf("%s\n",query);
  sprintf(query,"delete from Investments.options where stock_symbol = \"%s\"",Sym);
  if (!DEBUG) if (mysql_query(mysql,query)) puts("03 Failed to delete stock from Investments.options");
  if (DEBUG) printf("%s\n",query);

  // finished with the database
  #include "Includes/mysql-disconn.inc"
  printf("%s deleted\n",Sym);
  exit(EXIT_SUCCESS);
}
