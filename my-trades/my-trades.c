// my-trades.c
/* Parms: [yyyy-mm-dd] optional to run on prior dates
 * compile:  gcc -Wall -O2 -ffast-math my-trades.c -o my-trades `mysql_config --include --libs` `curl-config --libs`
 */

#include	"./my-trades.h"

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_date.inc"
#include	"../Includes/holiday_check.inc"
#include	"../Includes/Safe.inc"
#include	"../Includes/Chandelier.inc"
#include	"../Includes/BidAsk.inc"
#include	"../Includes/ParseData.inc"

int main(int argc, char *argv[]) {
  #include	"./my-trade-queries.h"
  char	thisDate[12];
  char query[1024];
  float	CurClose,CurLow,CurHigh;
  float R1,R2,R3,S1,S2,S3,PP;
  float	rsi;
  int	num_rows,x;
  int	Safe_Periods=20;
  int	Safe_Coeff=2;
  int	Safe_Stickyness=6;
  time_t t;
  struct tm *TM;
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  unsigned long	*lengths;

  // parse cli parms
  if (argc > 2) Usage(argv[0]);
  if (argc == 1) {
    t = time(NULL);
    TM = localtime(&t);
    if (TM == NULL) {
      perror("localtime");
      if (mysql != NULL) mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
    if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
      fprintf(stderr, "strftime returned 0");
      if (mysql != NULL) mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
  }
  if (argc==2) {
    if (sscanf(argv[1],"%1u%1u%1u%1u-%1u%1u-%1u%1u",&x,&x,&x,&x,&x,&x,&x,&x) == 8)
      strcpy(qDate, argv[1]);
    else Usage(argv[0]);
  }

 // connect to the database
  #include "../Includes/beancounter-conn.inc"
  holiday_check(qDate);
  strcpy(thisDate,qDate);
  // create the temp table
  if (mysql_query(mysql,create_table)) { print_error(mysql, "Failed to create temp table"); }
  if (mysql_query(mysql,query_list)) { print_error(mysql, "Failed to query database for symbols"); }
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed for symbols"); } 
    
//Loop through all Symbols to populate temp table
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"01 Skipping null row data for %s\n",row_list[0]); continue; }
    valid_date(row_list[0]);
    if (BullishOB(row_list[0])) continue;
    if (BullishKR(row_list[0])) continue;
    if (TrendUp(row_list[0])) continue;
  }	// end while Loop, temp table populated
  mysql_free_result(result_list);	// finished with all-syms

// process candidate symbols from temp table
  if (mysql_query(mysql,"select distinct SYMBOL from TRADES")) { print_error(mysql, "Failed to query database for symbols"); }
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed for symbols"); } 
//Big Loop through selected candidate Symbols
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"07 Skipping NULL data for %s\n",row_list[0]); break; }
  // Check that RSI < 80, else skip it
    rsi = RSI_check(row_list[0]);
    if (rsi>80) {	// check RSI value, skip if too high
      printf("Skipping %s for RSI>80\n", row_list[0]);
      delete_bad(row_list[0]);
      continue;
    }

  // check MACD for selected symbols
    if (MACD_check(row_list[0])) {
      printf("Skipping %s for MACD\n", row_list[0]);
      delete_bad(row_list[0]);
      continue;
    }

  // check High Volume for selected symbols
    if (HV_check(row_list[0])) {
      printf("Skipping %s for Volume\n", row_list[0]);
      delete_bad(row_list[0]);
      continue;
    }

  // calculate Chandelier values
    Chandelier(row_list[0],qDate);
    sprintf(query,"update TRADES set CHANUP = %.4f, CHANDN = %.4f where SYMBOL = \"%s\"",ChanUp,ChanDn,row_list[0]);
    if (mysql_query(mysql,query)) {
      print_error(mysql, "Failed to update symbol into temp database");
      delete_bad(row_list[0]);
      continue;
    }

  // calculate Pivot Point values
    sprintf(query, "select day_high, day_low, day_close from stockprices where symbol = \"%s\" order by date",row_list[0]);
    if (mysql_query(mysql,query)) {
      print_error(mysql, "Failed to query database");
      delete_bad(row_list[0]);
      continue;
    }
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) { 
      print_error(mysql, "store_results failed"); 
      delete_bad(row_list[0]);
      continue;
    } 
    // fetch the last row
    mysql_data_seek(result,mysql_num_rows(result)-1);
    row=mysql_fetch_row(result);
    if (row==NULL) { delete_bad(row_list[0]); fprintf(stderr,"NULL data found, skipping %s\n",row_list[0]); mysql_free_result(result); continue; }
    // check for nulls
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { delete_bad(row_list[0]); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",row_list[0]); continue; }
    if (!lengths[1]) { delete_bad(row_list[0]); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",row_list[0]); continue; }
    if (!lengths[2]) { delete_bad(row_list[0]); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",row_list[0]); continue; }
    // extract data
    CurHigh = strtof(row[0], NULL);
    CurLow = strtof(row[1], NULL);
    CurClose = strtof(row[2], NULL);
    mysql_free_result(result);
    PP = (CurHigh + CurLow + CurClose) / 3;
    R1 = (PP*2)-CurLow;
    S1 = (PP*2)-CurHigh;
    R2 = (PP*3)-(CurLow*2);
    S2 = (PP*3)-(CurHigh*2);
    R3 = (PP*2)+CurHigh-(CurLow*2);
    S3 = (PP*2)+CurLow-(CurHigh*2);
    // calculate and store values 
    sprintf(query,"update TRADES set PP = %.4f, R1 = %.4f, R2 = %.4f, S1 = %.4f, S2 = %.4f where SYMBOL = \"%s\"",PP,R1,R2,S1,S2,row_list[0]);
    if (mysql_query(mysql,query)) {print_error(mysql, "Failed to update symbol into temp database"); }

  // calculate SafeZone values
    sprintf(query,"select day_high,day_low from stockprices where symbol = \"%s\" order by date",row_list[0]);
    if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query database"); }
    result=mysql_store_result(mysql);	  // save the query results
    if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
    num_rows=mysql_num_rows(result);
    if (num_rows == 0)  {
      printf("0 rows found for symbol \"%s\"\n", row_list[0]);
      mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
    if (num_rows < (Safe_Periods+Safe_Stickyness+Safe_Coeff)) {
      printf("Too few rows for %s in SafeZone\n",row_list[0]);
      delete_bad(row_list[0]);
      continue;
    }
    mysql_free_result(result); 
    Safe(row_list[0],qDate);
    sprintf(query,"update TRADES set SAFEUP = %.4f, SAFEDN = %.4f where SYMBOL = \"%s\"",SafeUp,SafeDn,row_list[0]);
    if (SafeDn < SafeUp) { delete_bad(row_list[0]); continue; }
    else if (mysql_query(mysql,query)) {
      print_error(mysql, "Failed to update symbol into temp database");
      delete_bad(row_list[0]);
      continue;
    }

  // get Bid/Ask values
    BidAsk(row_list[0]);
    sprintf(query,"update TRADES set BID = %.4f, ASK = %.4f where SYMBOL = \"%s\"",Bid,Ask,row_list[0]);
    if (mysql_query(mysql,query)) { 
      print_error(mysql, "Failed to update symbol into temp database"); 
      delete_bad(row_list[0]);
      continue;      
    }
  }	// end Big Loop
  mysql_free_result(result_list);	// finished with TRADE list

  // print and email the results
  do_output(thisDate);
  printf("All functions completed\n");
  exit(EXIT_SUCCESS);
}
