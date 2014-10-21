/* time_check.c
 * part of update_all_stocks
 * check if requested date is a business holiday or weekend
 */
#include "update_all_stocks.h"
char	qDate[12];
time_t t,t2;
struct tm *TM2 = 0;

int time_check(void) {
//  char mDate[12];
  char	query[1024];
//  unsigned long *lengths;
  struct tm *TM;
  int	num_rows;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

  if (debug) puts("Running time check");
  // initialize the time structures with today
  t = t2 = time(NULL);
  TM2 = localtime(&t);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    return(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);
  // is this a business day?
  if (TM->tm_wday>0 
    && TM->tm_wday<6) 
  {
    // make sure it is 30 minutes past market close time on a business day
    if (TM->tm_hour<15 
      || (TM->tm_hour==15 && TM->tm_min<30)) {
	puts("No updates, market is still open today\n"); 
	if (!debug) return(EXIT_FAILURE);
    }
  }
  // back up a weekend to the previous Friday
  if (TM->tm_wday == 6) {
    t -= DAY_SECONDS; 
    TM = localtime(&t);
    strftime(qDate, sizeof(qDate), "%F", TM);	// save the adjusted date
  } else if (TM->tm_wday == 0) {
    t -= DAY_SECONDS * 2; 
    TM = localtime(&t);
    strftime(qDate, sizeof(qDate), "%F", TM);	// save the adjusted date
  } 
  // check for holidays
  #include "../Includes/beancounter-conn.inc"
  sprintf(query,"select last_bus_day from Investments.holidays where holiday = \"%s\"",qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  num_rows = mysql_num_rows(result);
  if (num_rows == 1) {	// holiday today, use last business day date
    if ((row=mysql_fetch_row(result))==NULL) {
      puts("oops - count() failed");
      return(EXIT_FAILURE);
    }
    strcpy(qDate,row[0]);	// point to last_bus_day
    strptime(qDate,"%F",TM);	// ensure t and TM are concurrent with qDate
    t = mktime(TM);
  }
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  if (debug) printf("qDate is %s\n",qDate);
  return(EXIT_SUCCESS);
}
