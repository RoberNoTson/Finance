// daily_div_list.c -- replaces daily_div_list.sh
/* list weekly upcoming exDiv stocks
 * runs in crontab each evening or anytime on demand
 * StartDate="today"+"1 day"
 * if today == "Fri,Sat" ; then StartDate="next Monday"
 * EndDate="next Tuesday"+"3 weeks"
 * 
 * query list and add Pivot,Safe,Chandelier and bid/ask values for symbol
 * DailyList=$HOME/Documents/Investments/daily-div-list.txt
 * XLSfile=$HOME/Documents/Investments/daily-div-list.xls
 * 
 * compile: OBE USE MAKE gcc -Wall -O2 -ffast-math daily_div_list.c -o daily_div_list `mysql_config --include --libs` `curl-config --libs`
 */

#include	"daily_div_list.h"
#include        "../Includes/Safe.inc"
#include        "../Includes/Chandelier.inc"
#include        "../Includes/BidAsk.inc"
#include	"../Includes/PP.inc"
#include	"../Includes/print_error.inc"
#include        "../Includes/holiday_check.inc"
#include	"../Includes/valid_date.inc"
#include	"../Includes/ParseData.inc"

int	main(int argc, char *argv[]) {
  FILE  *file;
  char  *filename=OUT_FILE;
  char  *perms="w+";
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  char	query[1024];
  char	query_list[200];
  char	*div_query1="select d.symbol, d.exDiv, d.value, b.day_close \
      from Investments.Dividends d, beancounter.stockprices b, beancounter.stockinfo c \
      where exDiv between \"";
  char	StartDate[12];	// date +%F --date="today"+"1 day", if Friday or Saturday use "next Monday"
  char	*div_query2="\" and \"";
  char	EndDate[12];	// date +%F --date="next Tuesday"+"2 weeks"
  char	*div_query3="\"  and yield > "MINYIELD" \
      and value > "MINVAL" \
      and d.symbol = b.symbol \
      and d.symbol = c.symbol \
      and c.active = true \
      and c.p_e_ratio is not null \
      and c.avg_volume > "MINVOLUME" \
      and b.day_close > "MINPRICE" \
      and b.date = (\
      select max(distinct date) from beancounter.stockprices) \
      order by exDiv;";
  char	*qURL1="http://download.finance.yahoo.com/d/quotes.csv?s=";
  char	*qURL2="&e=.csv&f=b3b2";
  char	qURL[1024];
  int	x;
  time_t t;
  struct tm *TM;
  CURL *curl;
  CURLcode	res=0;
  struct	MemStruct	chunk;

  // parse cli parms
  if (argc > 1) {
    printf("Usage:  %s  (no parms) \n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // get today's date
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
  
  // calculate StartDate
  if (TM->tm_wday == 5) t += DAY_SECONDS*3;
  else if (TM->tm_wday == 6) t += DAY_SECONDS*2;
  else t += DAY_SECONDS;
  TM = localtime(&t);
  if (strftime(StartDate, sizeof(StartDate), "%F", TM) == 0) {
    fprintf(stderr, "strftime returned 0");
    if (mysql != NULL) mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  // calculate EndDate
  if (TM->tm_wday == 2) t += DAY_SECONDS*SPAN;
  else if (TM->tm_wday == 3) t += DAY_SECONDS*(SPAN-1);
  else if (TM->tm_wday == 4) t += DAY_SECONDS*(SPAN-2);
  else if (TM->tm_wday == 5) t += DAY_SECONDS*(SPAN-3);
  else if (TM->tm_wday == 0) t += DAY_SECONDS*(SPAN+2);
  else t += DAY_SECONDS*(SPAN+1);
  TM = localtime(&t);
  if (strftime(EndDate, sizeof(EndDate), "%F", TM) == 0) {
    fprintf(stderr, "strftime returned 0");
    if (mysql != NULL) mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (DEBUG) printf("StartDate: %s\tEndDate: %s\n",StartDate,EndDate);
  // build Div query string
  sprintf(query_list,"%s%s%s%s%s",div_query1,StartDate,div_query2,EndDate,div_query3);
  if (DEBUG) printf("%s\n",query_list);
  
  // connect to the database
  #include "../Includes/beancounter-conn.inc"
    if (mysql_query(mysql,query_list)) {
    print_error(mysql, "Failed to query database");
  }
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) {
    print_error(mysql, "store_results failed");
  } 
  chunk.memory = calloc(1,1);
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  file=fopen(filename,perms);

  //Big Loop through all Symbols
  if (DEBUG) printf("Starting loop\n");
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue; }
    chunk.size = 0;
    if(curl) {
      sprintf(qURL,"%s%s%s",qURL1,row_list[0],qURL2);
      curl_easy_setopt(curl, CURLOPT_URL, qURL);
      res=curl_easy_perform(curl);
    } 
    if (!curl || res) {
      sleep(5);
      res=curl_easy_perform(curl);
      if (res) {
      printf("Oop!, Lost internet connection\n");
      free(chunk.memory);
      mysql_free_result(result_list);
      #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
      }
    }
    valid_date(row_list[0]);
    if (DEBUG) printf("Calculating values for %s using %s\n",row_list[0],qDate);
    // calculate the Pivot Point
    if ((Pivot(row_list[0],qDate)) == EXIT_FAILURE) { fprintf(stderr,"Skipping bad Pivot data for %s\n",row_list[0]); continue; }
    
    // get Chandelier values
    if ((Chandelier(row_list[0],qDate)) == EXIT_FAILURE) { fprintf(stderr,"Skipping bad Chandelier data for %s\n",row_list[0]); continue; }
    
    // get Safe values
    if ((Safe(row_list[0],qDate)) == EXIT_FAILURE) { fprintf(stderr,"Skipping bad Safezone data for %s\n",row_list[0]); continue; }

    // get Bid/Ask prices
    if ((BidAsk(row_list[0])) == EXIT_FAILURE) { fprintf(stderr,"Skipping bad BidAsk data for %s\n",row_list[0]); continue; }
    
    if (!DEBUG) fprintf(file,"%s\t%s\t$%s\t$%s\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\n",
    row_list[0],row_list[1],row_list[2],row_list[3],PP,S2,SafeUp,ChanUp,Bid,R2,SafeDn,ChanDn,Ask);
    if (DEBUG) printf("%s\t%s\t$%s\t$%s\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\n",
    row_list[0],row_list[1],row_list[2],row_list[3],PP,S2,SafeUp,ChanUp,Bid,R2,SafeDn,ChanDn,Ask);
  }	// end of Big Loop
  fclose(file);
  if (DEBUG) printf("Loop finished\n");
  
  // email results
  sprintf(query,"cat %s |mailx -s \"Dividend List\" mark_roberson@tx.rr.com",filename);
  if (!DEBUG) system(query);
  // output a XLS format file
  sprintf(query,"ssconvert %s ",filename);
  strncat(query,filename,strlen(filename)-3);
  strcat(query,"xls");
  if (!DEBUG) if ((x=system(query)) != 0) printf("Spreadsheet conversion failed due to error %d\n",WEXITSTATUS(x));
  
  // clean up memory
  curl_easy_cleanup(curl);
  free(chunk.memory);
  //finished with the database
  mysql_free_result(result_list);
  #include "../Includes/mysql-disconn.inc"
  if (DEBUG) printf("All functions completed\n");
  exit(EXIT_SUCCESS);
}
