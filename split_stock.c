/* split_stock.c
 * 
 * Parms:  Sym ratio(def. 2/1) yyyy-mm-dd(def. today)
 * Databases updated: 
 * 	beancounter.stockprices (previous_close,day_open,day_low,day_high,day_close,volume)
 * 	beancounter.portfolio (shares, cost)
 * 	Investments.activity(buy_shares/buy_price if sell_date is null)
 * 	Investments.portfolio (current_shares) 
 * 	Investments.watchlist(buy_shares/buy_price if sell_date is null)
 * Ex: split_stock KO 3/1 2012-08-01
 * Split-adjusts the stock (e.g. KO) by a factor of 3 for each 1 share: 
 * price data in the database is divided by three, volume increased by 3 and, 
 * in the portfolios, owned shares are increased and cost is decreased.
 * 
 * Simple ratios can be entered as just the numerator (e.g.  split_stock KO 3 (equivalant to 3/1) )
 * Ratio of 3/2 works similarly with a factor of 3 for each 2 shares (alt. can use 1.5 or 1.5/1 )
 * 
 * Date is the last day at the pre-split price. 
 * For example, run on Friday night (or over the weekend) when the stock will open split the following Monday.
 * 
 * compile: gcc -Wall -O2 -ffast-math split_stock.c -o split_stock `mysql_config --include --libs`
 */

// set DEBUG to 1 for testing, 0 for production
#define         DEBUG   0
#define         _XOPEN_SOURCE
#define         _XOPENSOURCE

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <unistd.h>
#include        <curl/curl.h>
#include        <ctype.h>

MYSQL *mysql;
char	qDate[16];

#include        "Includes/print_error.inc"
#include        "Includes/valid_sym.inc"
//#include        "Includes/valid_date.inc"

void    Usage(char *prog) {
  printf("\nUsage:  %s Sym [ratio(def. 2/1)] [yyyy-mm-dd(def. today)]\n", prog);
  printf("\nExample: split_stock KO 3/1 2012-08-01\n \
  The example split-adjusts the stock (KO) by a factor of 3 for each 1 share,\n \
  effective through the given date.\n \
  Price data in the database is divided by three, volume increased by 3 and,\n \
  in the portfolios, owned shares are increased and cost is decreased.\n\n \
  Simple ratios can be entered as just the numerator\n \
  (e.g.  split_stock KO 3 (equivalant to 3/1) )\n \
  Ratio of 3/2 works similarly with a factor of 3 for each 2 shares\n \
  (alternatively, use 1.5 or 1.5/1 )\n \
  The default ratio is 2/1\n\n \
  Date is the last day at the pre-split price, defaults to today.\n \
  For example, \"split_stock KO\"\n \
  should be run on Friday night (or over the weekend)\n \
  when the stock will open split 2 for 1 on the following Monday.\n \
  Databases updated:\n \
 	beancounter.stockprices (previous_close,day_open,day_low,day_high,day_close,volume)\n \
 	beancounter.portfolio (shares, cost)\n \
 	Investments.activity(buy_shares/buy_price if sell_date is null)\n \
 	Investments.portfolio (current_shares)\n \
 	Investments.watchlist(buy_shares/buy_price if sell_date is null)\n\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char * argv[]) {
  char  query[1024];
  char  Sym[16];
  int	x;
  float	Ratio=0.5;
  float	SharesAdj=2;
  float	Num=2,Denom=1;
  time_t t;
  struct tm *TM = 0;

  // parse cli parms
  if (argc < 2 || argc > 4) Usage(argv[0]);
  for (x=0; x<strlen(Sym);x++); Sym[x]=0;
  for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
  if (DEBUG) printf("Processing symbol %s\n",Sym);
  // parse time/date
  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);
  // parse the ratio
  if (argc > 2) {
    if (strchr(argv[2],'/')) {
      sscanf(argv[2],"%f%*[/]%f",&Num,&Denom);
      Ratio = Denom / Num;
      SharesAdj = Num / Denom;
    } else {
      Ratio = 1 / strtof(argv[2],NULL);
      SharesAdj = strtof(argv[2],NULL);
    }
  }
  if (DEBUG) printf("Ratio: %.4f\tSharesAdj: %.4f\n",Ratio,SharesAdj);

  if (argc == 4) {
    // parse the split date
    if (sscanf(argv[3],"%1u%1u%1u%1u-%1u%1u-%1u%1u",&x,&x,&x,&x,&x,&x,&x,&x) == 8)
      strcpy(qDate, argv[3]);
    else Usage(argv[0]);
  }
  #include "Includes/beancounter-conn.inc"
  valid_sym(Sym);
//  valid_date(Sym);
  if (DEBUG) printf("Using split date: %s\n",qDate);
  // verify the change is really correct
  puts("The following change has been requested:");
  printf("Split %s by a ratio of %.1f for every %.1f shares owned,\neffective after COB on %s\n",Sym,Num,Denom,qDate);
  printf("Are you sure this is correct? [y/N]: ");
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  if (getchar() != '\n') {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  printf("Splits cannot be undone, are you absolutely sure the values are correct? [y/N]: ");
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  if (getchar() != '\n') {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }

  // beancounter database updates
  sprintf(query,"update stockprices set previous_close = previous_close * %.4f, \
    day_open = day_open * %.4f, \
    day_low = day_low * %.4f, \
    day_high = day_high * %.4f, \
    day_close = day_close * %.4f, \
    day_change = day_change * %.4f, \
    volume = volume * %.4f \
    where symbol = \"%s\" and date <= \"%s\"",Ratio,Ratio,Ratio,Ratio,Ratio,Ratio,SharesAdj,Sym,qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) printf("01 Failed to update %s split in beancounter.stockprices database\n",Sym);

  sprintf(query,"update portfolio set shares = shares * %.4f, cost = cost * %.4f \
    where symbol = \"%s\" and date <= \"%s\"",SharesAdj,Ratio,Sym,qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) printf("01 Failed to update %s split in beancounter.portfolio database\n",Sym);
  
  // Investments database updates
  sprintf(query,"update Investments.activity set buy_price = buy_price * %.4f, \
    buy_shares = buy_shares * %.4f \
    where symbol = \"%s\" and sell_date is null and buy_date <= \"%s\"",Ratio,SharesAdj,Sym,qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) printf("03 Failed to update %s split in Investments.activity database\n",Sym);
  
  sprintf(query,"update Investments.portfolio set current_shares = current_shares * %.4f \
    where symbol = \"%s\"and current_shares > 0",SharesAdj,Sym);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) printf("03 Failed to update %s split in Investments.portfolio database\n",Sym);

  sprintf(query,"update Investments.watchlist set PAPER_BUY_PRICE = PAPER_BUY_PRICE * %.4f \
    where SYMBOL = \"%s\" and PAPER_BUY_PRICE is not null and PAPER_SELL_PRICE is null \
    and date <= \"%s\"", Ratio,Sym,qDate);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if (mysql_query(mysql,query)) printf("03 Failed to update %s split in Investments.watchlist database\n",Sym);
  
  #include "Includes/mysql-disconn.inc"
  printf("Split processed for %s\n",Sym);
  exit(EXIT_SUCCESS);
}
