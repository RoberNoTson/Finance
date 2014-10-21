// buy_stock.c
/* insert new stock data into the database
 * 
 * Parms: [Sym] (menu driven interface)
 * compile:  gcc -Wall -O2 -ffast-math buy_stock.c -o buy_stock `mysql_config --include --libs`
 */

#define	DEBUG	0
#define		_XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<errno.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"
#include        "Includes/valid_sym.inc"

int	main (int argc, char *argv[]) {
  char	qDate[12];
  char	buy_date[12];
  char	prevDate[12];
  char	Sym[16];
  char	query[1024];
  int	Shares;
  int	current_shares=0;
  float	Price;
  float	Comm;
//  float	Fees;
  float	PL=0.0;
  int	x,num_rows;
  MYSQL_RES *result;
  MYSQL_ROW row=NULL;
  time_t t;
  struct tm *TM = 0;

  if (argc > 2) {
    printf("Usage: %s [symbol]\n\tMenu/Response system, optionally can pass \"symbol\" on CLI\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  memset(Sym,0,sizeof(Sym));
  memset(qDate,0,sizeof(qDate));
  // initialize the time structures with today
  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);

  // get Sym if not passed on CLI
  if (argc==2) strcpy(query,argv[1]);
  else {
    printf("Ticker symbol: ");
    errno=0;
    if ((x=scanf("%s",query)) == 0) print_error(mysql,"No symbol selection made\n");
    else if (errno)  print_error(mysql,"Read error\n");
    else if (x == EOF) print_error(mysql,"No identifiable selection made\n");
  }
  for (x=0;x<strlen(query);x++) Sym[x]=toupper(query[x]);
  if (DEBUG) printf("Symbol: %s\n",Sym);
  #include "Includes/beancounter-conn.inc"
  valid_sym(Sym);
  printf("Number of shares bought [100]: ");
  if (argc==1) query[0]=getchar();
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  errno=0;
  if (!strlen(query)) Shares=100;
  else Shares=strtol(query,NULL,10);
  if (errno) print_error(mysql,"Invalid value");
  if (DEBUG) printf("Shares: %d\n",Shares);
  
// get price of shares sold - no default
  printf("Price of shares: ");
  errno=0;
  if ((x=scanf("%f",&Price)) == 0) print_error(mysql,"No selection made\n");
  else if (errno)  print_error(mysql,"Read error\n");
  else if (x == EOF) print_error(mysql,"No identifiable selection made\n");
  if (DEBUG) printf("Price: %.4f\n",Price);
  
// get purchase date (default today)
  printf("Buy date [%s]: ",qDate);
  buy_date[0]=getchar();
  fgets(buy_date,sizeof(buy_date),stdin);
  if (strchr(buy_date,'\n')) memset(strchr(buy_date,'\n'),0,1);
  if (!strlen(buy_date)) strcpy(buy_date,qDate);
  if (DEBUG) printf("Date: %s\n",buy_date);
  
// get commission and fees - default 0.00
  printf("Broker commision [0.00]: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) Comm=0.00;
  else Comm=strtof(query,NULL);
  if (DEBUG) printf("Comm: %.2f\n",Comm);
  // last verification before commit
  printf("All values correct? [y/n]");
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  
  // query for already held shares of the stock
  sprintf(query,"select current_shares, PL \
    from Investments.portfolio \
    where symbol = \"%s\" \
    and current_shares > 0",Sym);
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) print_error(mysql,"01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows) {
    row=mysql_fetch_row(result);
    if(row == NULL) print_error(mysql,"02 Failed to query database"); 
    current_shares=atoi(row[0]);
    PL=strtof(row[1],NULL);
  } else {
    current_shares=0;
    PL=0;
  }
  mysql_free_result(result);

  // is it already in portfolio?
  sprintf(query,"select distinct symbol from Investments.portfolio where symbol = \"%s\"",Sym);
  mysql_query(mysql,query);
  result=mysql_store_result(mysql);
  if (!mysql_num_rows(result)) {
    // no, add it
    sprintf(query,"insert into Investments.portfolio (symbol) values (\"%s\")",Sym);
    if (!DEBUG) if (mysql_query(mysql,query)) { print_error(mysql, "Failed to insert new symbol into Investments.portfolio"); }
  }
  mysql_free_result(result);

  // update Investments.portfolio
  sprintf(query,"insert into Investments.activity (symbol,buy_date,buy_price,buy_shares,buy_fees) \
    values(\"%s\",\"%s\",%.4f,%d,%.2f)", Sym,buy_date,Price,Shares,Comm);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"02 Failed to update activity database");
  sprintf(query,"update Investments.activity set Lot_id = id where id = %d",(int)mysql_insert_id(mysql));
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"03 Failed to update Lot_id");
  
  // update beancounter
  sprintf(query,"insert into beancounter.cash (value,type,cost,date,name) values(%.2f,\"Buy\",%.2f,\"%s\",\"ML-CMA\")",
	  -(Shares*Price),Comm,buy_date);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"Failed to update cash database");
  sprintf(query,"insert into beancounter.portfolio (symbol,shares,cost,date) values(\"%s\",%d,%.2f,\"%s\")",
	  Sym,Shares,Price,buy_date);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"03 Failed to update beancounter.portfolio database");

  // update watchlist where it matches
  sprintf(query,"select distinct(date) from Investments.watchlist where date < \"%s\" and SYMBOL = \"%s\" order by date desc limit 1",buy_date,Sym);
  if (mysql_query(mysql,query)) print_error(mysql,"04 Failed to query database for dates");
  if ((result=mysql_store_result(mysql)) && (mysql_num_rows(result))) {
    row=mysql_fetch_row(result);
    strcpy(prevDate,row[0]);
    mysql_free_result(result);
    sprintf(query,"update Investments.watchlist set REAL_BUY_PRICE = %.4f where SYMBOL = \"%s\" \
    and date = \"%s\" and REAL_BUY_PRICE is null",Price,Sym,prevDate);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if (mysql_query(mysql,query)) print_error(mysql,"04 Failed to update watchlist database for Buy");
  }
  
  puts("Database updated");
    // finished with the database
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
