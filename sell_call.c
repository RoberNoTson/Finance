/* sell_call.c
 * update beancounter.cash by adding a value from Call Option sale
 * 
 * Parms: [0.00] (will prompt if needed)
 * compile: gcc -Wall -O2 -ffast-math sell_call.c -o sell_call `mysql_config --include --libs`
 */

#define	DEBUG	0
#define		_XOPEN_SOURCE

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
  char	qDate[12];
  char	opt_date[12];
  char	exp_date[12];
  char	OptSym[20];
  char	Sym[16];
  char	query[1024];
  int	x;
  int	num_con=0;
  float	value=0.0;
  float	Fees=0.0;
  float strike_price=0.0;
  time_t t;
  struct tm *TM = 0;

  if (argc > 2) {
    printf("Usage: %s [OptSym]\n",argv[0]);
    puts("Pass the Option Contract symbol to add, or it will prompt for it.");
    exit(EXIT_FAILURE);
  }
  memset(Sym,0,sizeof(Sym));
  memset(OptSym,0,sizeof(OptSym));
  memset(query,0,sizeof(query));
  memset(qDate,0,sizeof(qDate));
    // initialize the time structures with today
  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);
  if (argc==2) { for (x=0;x<strlen(argv[1]);x++) OptSym[x]=toupper(argv[1][x]); }
  else {
    // prompt for Option symbol
    printf("Option Contract symbol: ");
    fgets(OptSym,sizeof(OptSym),stdin);
    if (strchr(query, '\n')) memset(strchr(query,'\n'),0,1);
    if (!strlen(OptSym)) {
      puts("No Option contract symbol, exiting unchanged");
      exit(EXIT_FAILURE);
    }
  }
  if (strlen(OptSym) < 6) {
    puts("Not a valid contract symbol, exiting unchanged");
    exit(EXIT_FAILURE);
  }
  x=0;
  while(isalpha(OptSym[x])) {
    Sym[x] = OptSym[x];
    x++;
  }
  // prompt for the underlying symbol
  printf("Underlying stock symbol [%s]: ", strlen(Sym)? Sym : "");
  fgets(query, sizeof(query), stdin);
  if (strchr(query, '\n')) memset(strchr(query,'\n'),0,1);
  if (strlen(query)) strcpy(Sym, query);
  // get strike price
  printf("Option strike price: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) {
      puts("No strike price found, exiting unchanged");
      exit(EXIT_FAILURE);
  }
  strike_price = fabsf(strtof(query,NULL));
  // prompt for the number of contracts
  printf("Number of contracts sold: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query))  if (num_con == 0) {
    puts("No contracts entered, exiting unchanged");
    exit(EXIT_FAILURE);
  }
  num_con = atoi(query);
  printf("Amount received per contract: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) {
    puts("No value found, exiting unchanged");
    exit(EXIT_FAILURE);
  }
  value = fabsf(strtof(query,NULL));
  value *= num_con;
  value *= 100;
  
  printf("Total all fees: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query))  {
      puts("No Fees found, exiting unchanged");
      exit(EXIT_FAILURE);
  }
  Fees = fabsf(strtof(query,NULL));

  printf("Option transaction date [%s]: ",qDate);
  fgets(opt_date,sizeof(opt_date),stdin);
  if (strchr(opt_date,'\n')) memset(strchr(opt_date,'\n'),0,1);
  if (!strlen(opt_date)) strcpy(opt_date,qDate);
  if (DEBUG) printf("Date: %s\n",opt_date);
  printf("Option expiration date: ");
  fgets(exp_date,sizeof(exp_date),stdin);
  if (strchr(exp_date,'\n')) memset(strchr(exp_date,'\n'),0,1);
  if (!strlen(exp_date)) strcpy(exp_date,qDate);
  if (DEBUG) printf("Expiration Date: %s\n",exp_date);

  // last verification before commit
  printf("Verify this:  Sold %d contracts totaling $%.2f less fees of $%.2f on %s\n",num_con, value,Fees,opt_date);
  printf("All values correct? [y/N]");
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  #include	"Includes/beancounter-conn.inc"
  sprintf(query,"insert into beancounter.cash () values(\"ML-CMA\",\"%.2f\",DEFAULT,\"Option\",DEFAULT,\"%.2f\",\"%s\")", value, Fees, opt_date);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"Failed to update database");
  memset(query,0,sizeof(query));
  sprintf(query, "insert into Investments.options (symbol, stock_symbol, date, exp_date, strike) values(\"%s\",\"%s\",\"%s\",\"%s\",\"%.2f\")",OptSym,Sym, qDate, exp_date, strike_price);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"Failed to update database");

  puts("Database updated with new Option cash value");
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
