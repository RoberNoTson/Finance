// mt2.c -- my_trades part 2
/* 1) update prior session selections with current data to calculate new sell prices
 * 2) also updates PAPER_BUY where day_low <= MEDIAN_BUY
 * and PAPER_SELL and PAPER_PL where day_high exceeds AVG_SELL_NEXT for the next day.
 * The price might have continued up past this point, but we are only interested in meeting the prediction.
 * 
 * NOTE: there is still a hole here if a same-day trade occurred for the MEDIAN_BUY/MEDIAN_SELL combo
 * NOTE: need to add stop-loss prices to minimize the downside
 * 
 * Parms: [date]
 * compile: make
 */

#include	"./mt2.h"

#include	"../Includes/print_error.inc"
double	PP,R1,R2,R3,S1,S2,S3;
#include	"../Includes/EnhPivot.inc"
double	Bid,Ask;
#include	"../Includes/BidAsk.inc"
double	ChanUp,ChanDn;
#include	"../Includes/Chandelier.inc"
double	SafeUp,SafeDn;
#include	"../Includes/Safe.inc"

char	qDate[12];	// used for calculating most current values
char	priorDate[12];	// date of potential BUY trades
char	prevpriorDate[12];	// date of potential SELL trades

int main (int argc, char *argv[]) {
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  char	thisSym[12];
  char	query[1024];
  unsigned long *lengths;
  int	num_rows;

  // parse CLI parms
  if (argc > 2) Usage(argv[0]);
  if (argc == 2) { printf("Date Function not yet enabled in this version\n"); exit(EXIT_FAILURE); }
  
  // get current and prior dates from beancounter.stockprices 
  #include "../Includes/beancounter-conn.inc"
  sprintf(query,"select distinct(date) from %s.stockprices order by date desc limit 3",DB_BEANCOUNTER);
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

  // select all symbols from prior date for Loop
  sprintf(query,"select distinct(SYMBOL) from %s.watchlist where date = \"%s\"",DB_INVESTMENTS,priorDate);
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  if ((result_list=mysql_store_result(mysql)) == NULL) print_error(mysql,"MT2 store result_list failed");
  if ((num_rows=mysql_num_rows(result_list))==0)  print_error(mysql,"No tickers found for prior date, exiting\n"); 
  if (DEBUG) printf("%d rows found\n",num_rows);

  // loop to update watchlist Pivot, Chand, Safe, Ask for paper-trades
  while ((row_list=mysql_fetch_row(result_list)) != NULL) {
    if(row_list == NULL) { fprintf(stderr,"01 Skipping null row data in MT2\n"); continue; }
    strcpy(thisSym,row_list[0]);
    // calculate R1_NEXT, CHANDN_NEXT, SAFEDN_NEXT
    if (Pivot(thisSym,qDate) == EXIT_FAILURE) {
      printf("Pivot failed for %s\n",thisSym);
      PP=R1=R2=S1=S2=0;
      continue;
    }
    if (DEBUG) printf("PP %.4f\tR1 %.4f\t R2 %.4f\t S1 %.4f\t S2 %.4f\t \n",PP,R1,R2,S1,S2);
    Chandelier(thisSym,qDate);
    if (DEBUG) printf("ChanUp %.4f\tChanDn %.4f\n",ChanUp,ChanDn);
    Safe(thisSym,qDate);
    if (DEBUG) printf("SafeUp %.4f\tSafeDn %.4f\n",SafeUp,SafeDn);
    // download ASK_NEXT
    BidAsk(thisSym);
    if (DEBUG) printf("Bid %.2f\tAsk %.2f\n",Bid,Ask);
    // update watchlist
    sprintf(query,"update %s.watchlist set R1_NEXT=%.4F,CHANDN_NEXT=%.4F,SAFEDN_NEXT=%.4F,ASK_NEXT=%.4F \
    where SYMBOL = \"%s\" and date = \"%s\"",DB_INVESTMENTS,R1,ChanDn,SafeDn,Ask,thisSym,priorDate);
    if (!DEBUG) mysql_query(mysql,query);
    if (DEBUG) printf("%s\n",query);
  } // end Loop
  mysql_free_result(result_list);

  // update watchlist paper-trades for buy/sell/stop
  do_updates();

  // print daily P/L report for paper trades
  do_output();

  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  
  printf("All functions completed\n");
  exit(EXIT_SUCCESS);
}
