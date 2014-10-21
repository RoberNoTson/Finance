/* time_check.c
 * part of watchlist_monitor
 * check if requested date is a business holiday or weekend
 */
#include "watchlist_monitor.h"

int time_check(void) {
  char mDate[12];
  unsigned long	*lengths;
  time_t t;
  struct tm *TM=0;
  char	*saveptr;
  int	mkt_open=0;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    return(EXIT_FAILURE);
  }
  if (strftime(mDate, sizeof(mDate), "%F", TM) == 0) {
    puts("In time_check -  strftime returned 0");
    return(EXIT_FAILURE);
  }
  // skip weekends
  if (TM->tm_wday < 1 || TM->tm_wday > 5) {
    puts("Today is a weekend, aborting processing");
    if (!debug) mysql_close(mysql);
    if (!debug) return(EXIT_FAILURE);
  }
  #include "../Includes/beancounter-conn.inc"
  // check for and skip business holidays
  if (mysql_query(mysql,"select distinct(bus_holiday) from Investments.holidays where bus_holiday <= CURRENT_DATE order by bus_holiday")) {
    puts("Error parsing holidays");
    #include "../Includes/mysql-disconn.inc"
    if (!debug) return(EXIT_FAILURE);
  }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) {	// no results returned, 
    puts("Holiday store_results failed");
    return(EXIT_FAILURE);
  } 
  if ((int)mysql_num_rows(result) == 0)  {
    puts("0 rows found for Holidays, exiting.");
    mysql_close(mysql);
    return(EXIT_FAILURE);
  }
  // fetch the row
  while ((row=mysql_fetch_row(result))) {
    if (row == NULL) {
      puts("No data rows fetched for Holiday check, exiting.");
      mysql_close(mysql);
      return(EXIT_FAILURE);
    }
    if (row[0] == NULL) { mysql_free_result(result); puts("Invalid date or fetch of row failed"); return(EXIT_FAILURE); }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); puts("Invalid date or fetch of row failed"); return(EXIT_FAILURE); }
    if ((strcmp(row[0],mDate))==0) {
      printf("%s is a market holiday, aborting processing\n",mDate);
      if (!debug) mysql_close(mysql);
      if (!debug) return(EXIT_FAILURE);
    }
  }
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  
  // check the time
  t = time(NULL);
  if ((TM = localtime(&t)) == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
  mkt_open=0;
  if (TM->tm_hour>strtoul(market_close,&saveptr,10) || 
    (TM->tm_hour==strtoul(market_close,&saveptr,10) && TM->tm_min>(strtoul(saveptr+1,NULL,10)+(interval/60)))) {
    // market is closed for the day
    printf("Stock market is closed for the day (after %s)\n",market_close);
    if (!debug) return(EXIT_FAILURE);
  }
  while (TM->tm_hour<strtoul(market_open,&saveptr,10) || 
    (TM->tm_hour==strtoul(market_open,&saveptr,10) && TM->tm_min<strtoul(saveptr+1,NULL,10))) {
    // too early, should we hang around?
    if (!mkt_open) printf("Stock market not open yet, waiting until %s\nPress 'Q' to Quit, 'H' for Help\n",market_open);
    if (!mkt_open) mkt_open = 1;
    sleep(interval);
    t = time(NULL);
    if ((TM = localtime(&t)) == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
  } 
  if (!mkt_open) mkt_open = 1;
  return 0;
}
