// sell_stock.c
/* update database when shares are sold
 * NOTE: tbd - add procedure to select multiple holdings when sale is greater than any one buy
 * 
 * Parms: [Sym] - menu driven input
 * compile:  gcc -Wall -O2 -ffast-math sell_stock.c -o sell_stock `mysql_config --include --libs`
 */
#include	"sell_stock.h"
#include        "../Includes/print_error.inc"

  char	Sym[16];
  char	buy_date[12];
  char	sDate[12];
  int	Shares;
  int	buy_shares;
  int	holdings;
  int	updated_holdings;
  int	ID;
  float	buy_price;
  float	Price=0;
  float	Comm=0;
  float	Fees=0;

int	main (int argc, char *argv[]) {
  int	x;
  time_t t;
  struct tm *TM = 0;


  // parse CLI parms
  memset(Sym,0,sizeof(Sym));
  if (argc==2) { for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]); }
  
  // initialize the time structures with today
  memset(qDate,0,sizeof(qDate));
  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);
  
  #include "../Includes/beancounter-conn.inc"
  if (create_menu() == EXIT_FAILURE) {
    // finished with the database
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

  if (get_input() == EXIT_FAILURE) {
    // finished with the database
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

  // Update databases. Activity table changes will trigger auto updates of portfolio table.
  if (update_Investments_db() == EXIT_FAILURE) {
    // finished with the database
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  if (update_watchlist() == EXIT_FAILURE) {
    // finished with the database
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  if (update_beancounter_db() == EXIT_FAILURE) {
    // finished with the database
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  
  puts("Database updated");
  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
