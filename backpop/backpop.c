// backpop.c -- back popluate pricing data in the beancounter.stockprices table through most recent available
/* NOTE: this will overwrite existing data
 * NOTE: Yahoo doesn't update the history tables for today's data until later, sometimes after midnight Eastern time,
 * 	so we try to use yahooquote data for today if the market is closed
 * 
 * Parms: Sym [prev-date (default 1 year ago)]
 * compile:  gcc -Wall -O2 -ffast-math -o backpop backpop.c `mysql_config --include --libs` `curl-config --libs`
 * 
 */

#include	"./backpop.h"

#include        "../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"
time_t t,t2;
struct tm *TM;
struct tm *TM2;
int updated=0;
  
int main(int argc, char * argv[]) {
  char	qDate[12];
  char	thisDate[12];
  char	query[1024];
  char	qURL[1024];
  char	Sym[16];
  char	buf[1024];
  char	day_open[32]="DEFAULT";
  char	day_close[32]="DEFAULT";
  char	day_high[32]="DEFAULT";
  char	day_low[32]="DEFAULT";
  char	day_change[32]="DEFAULT";
  char	volume[32]="DEFAULT";
  char	prev_close[32]="DEFAULT";
  char	exchange[32]="DEFAULT";
  char	capitalisation[32]="DEFAULT";
  char	low_52weeks[32]="DEFAULT";
  char	high_52weeks[32]="DEFAULT";
  char	earnings[32]="DEFAULT";
  char	dividend[32]="DEFAULT";
  char	p_e_ratio[32]="DEFAULT";
  char	avg_volume[32]="DEFAULT";
  char	*saveptr2;
  int num_rows,x;

  
  // parse cli parms
  if (argc == 1 || argc >3) Usage(argv[0]);
  // convert symbol parm to uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  // verify symbol exists in stockinfo, else exit
  #include "../Includes/beancounter-conn.inc"
  valid_sym(Sym);
  
  // initialize the time structures with today
  t = time(NULL);
  TM = localtime(&t);
  TM2 = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  if (argc == 2) {
    // point to last year
    TM->tm_year--;
    strftime(qDate, sizeof(qDate), "%F", TM);
    t = mktime(TM);
  }
  
  // Validate the CLI parm date
  if (argc == 3) {
    // is it a valid date format?
    if (sscanf(argv[2],"%1u%1u%1u%1u-%1u%1u-%1u%1u",&x,&x,&x,&x,&x,&x,&x,&x) != 8) Usage(argv[0]);
    // date parm passed, process it
    strcpy(qDate, argv[2]);
    // validate qDate
    strptime(qDate,"%F",TM);
    t = mktime(TM);
    strftime(buf,sizeof(buf),"%F",TM);
    // if dates do not match, reject it
    if (strcmp(qDate,buf)) {
      fprintf(stderr, "Invalid date: input %s converted to %s\n",qDate,buf);
      exit(EXIT_FAILURE);
    }
    // was today's date passed? 
    t2 = time(NULL);
    TM = localtime(&t2);
    strftime(buf,sizeof(buf),"%F",TM);
    if ((strncmp(qDate,buf,10)==0)) {
      // yes, make sure it is after market closing time
      if (TM->tm_hour<15 || (TM->tm_hour==15 && TM->tm_min<30)) {
	printf("No data found for %s, market is still open today\n",Sym); 
	exit(EXIT_FAILURE);
      }
    }
    // see if a future date was supplied, reject it
    if (t > time(NULL)) {
      fprintf(stderr, "Invalid date: %s is in the future\n",qDate);
      exit(EXIT_FAILURE);
    }
  }	// end If argc==3
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
  sprintf(query,"select last_bus_day from Investments.holidays where holiday = \"%s\"",qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  num_rows = mysql_num_rows(result);
  mysql_free_result(result);
  if (num_rows == 1) {
    if ((row=mysql_fetch_row(result))==NULL) {
      printf("oops - count() failed\n");
      exit(EXIT_FAILURE);
    }
    strcpy(qDate,row[0]);	// point to last_bus_day
  }
  // ensure t and TM are concurrent with qDate
  strptime(qDate,"%F",TM);
  t = mktime(TM);
  
  // main processing
  // this will exit and not return if valid data is found
  get_history(Sym,qDate);
  if (DEBUG) printf("No history data was updated, attempting to get today's data\n");

  // history not updated, check if force of today is requested
  // was today's date passed? 
  t2 = time(NULL);
  TM = localtime(&t2);
  strftime(buf,sizeof(buf),"%F",TM);
  if ((strncmp(qDate,buf,10)==0)) {
    // yes, make sure it is after market closing time
    if (TM->tm_hour<15 || (TM->tm_hour==15 && TM->tm_min<30)) {
      printf("No data found for %s, market is still open today\n",Sym); 
      exit(EXIT_FAILURE);
    }
  }
  
  // use YahooQuote to get today's data, local time > 15:30
  // get date,close,open,prev_close,volume,change,low-high
  sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=d1l1opvc1mxj1edra2w",Sym);
  chunk.size=0;
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
  res=curl_easy_perform(curl);
  if (res) {	// error, no data retrieved
    if (res == 56 || res == 28) {	// retry
	res=curl_easy_perform(curl);
	if (res) {
	  printf("Error %d Unable to access the internet\n",res);
	  printf("%s\n",errbuf);
	  exit(EXIT_FAILURE);
	}
    } else {	// retry
      printf("Error %d Unable to access the internet\n",res);
      printf("%s\n",errbuf);
      print_error(mysql,"curl error, no data retrieved");
      exit(EXIT_FAILURE);
    }
  }
  // was any data returned?
  if (strstr(chunk.memory,"404 Not Found")) { 
    printf("%s not found at YahooQuote for today, \"404\" error\n",Sym);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

  // parse the price data for today
  memset(thisDate,0,sizeof(thisDate));
  memset(TM2,0,sizeof(struct tm));
  sscanf(chunk.memory,"%*[\"]%[0-9/NA]%*[\",]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9/NA]%*[,]%[0-9.-+/NA]%*[\",]%[0-9./NA]%*[ -]%[0-9./NA]%*[\",]%[A-Za-z/]%*[\",]%[0-9.BMTK/NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9/NA]%*[\",]%[0-9./NA]%*[ -]%[0-9./NA]%*s",
    thisDate,day_close,day_open,prev_close,volume,day_change,day_low,day_high,exchange, capitalisation,earnings,dividend,p_e_ratio,avg_volume,low_52weeks,high_52weeks);
  if (DEBUG) printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
   thisDate,day_close,day_open,prev_close,volume,day_change,day_low,day_high,exchange, capitalisation,earnings,dividend,p_e_ratio,avg_volume,low_52weeks,high_52weeks);

  // is the symbol valid?
  if (!strcmp(thisDate,"N/A")) {	// bad symbol, deactivate it
    printf("Bad symbol %s, deactivated\n",Sym);
    sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
    mysql_query(mysql,query);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  // verify the date is today
  // adjust the format to match ISO standard
  if (!strptime(thisDate,"%m/%d/%Y",TM2))  printf("strptime failed\n");
  strftime(buf,sizeof(buf),"%F",TM2);
  if (strncmp(qDate,buf,10)) {
    printf("No current data for %s since %s\n",Sym,buf);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  // split out info for today
  // early out if no Close price for today
    if (!strcmp(day_close,"0.00") || !strcmp(day_close,"N/A")) {
      printf("symbol %s has no close data\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
    if (!strcmp(day_open,"N/A")) strcpy(day_open,"DEFAULT");
    if (!strcmp(prev_close,"N/A")) strcpy(prev_close,"DEFAULT");
    if (!strcmp(volume,"N/A")) strcpy(volume,"DEFAULT");
    if (!strcmp(day_change,"N/A")) strcpy(day_change,"DEFAULT");
    if (!strcmp(day_low,"N/A")) strcpy(day_low,"DEFAULT");
    if (!strcmp(day_high,"N/A")) strcpy(day_high,"DEFAULT");
    if (!strcmp(exchange,"N/A")) strcpy(exchange,"DEFAULT");
    if (!strcmp(earnings,"N/A")) strcpy(earnings,"DEFAULT");
    if (!strcmp(dividend,"N/A")) strcpy(dividend,"DEFAULT");
    if (!strcmp(p_e_ratio,"N/A")) strcpy(p_e_ratio,"DEFAULT");
    if (!strcmp(avg_volume,"N/A")) strcpy(avg_volume,"DEFAULT");
    if (!strcmp(high_52weeks,"N/A")) strcpy(high_52weeks,"DEFAULT");
    if (!strcmp(low_52weeks,"N/A")) strcpy(low_52weeks,"DEFAULT");
    strcpy(buf,capitalisation);	// save capitalisation
    // convert capitalisation format to millions
    if ((saveptr2=strchr(buf,'B'))!=NULL) {	// convert Billions to Millions
	memset(saveptr2,0,1);
	sprintf(capitalisation,"%.4f",strtod(buf,NULL)*1000);
    } else if ((saveptr2=strchr(buf,'M'))!=NULL) {	// already in Millions
	memset(saveptr2,0,1);
	strcpy(capitalisation,buf);
    } else if ((saveptr2=strchr(buf,'T'))!=NULL) {	// convert Trillions to Millions
	memset(saveptr2,0,1);
	sprintf(capitalisation,"%.4f",strtod(buf,NULL)*1000000);
    } else if ((saveptr2=strchr(buf,'K'))!=NULL) {	// convert Thousands to Millions
	memset(saveptr2,0,1);
	sprintf(capitalisation,"%.4f",strtod(buf,NULL)*0.001);
    } else {	// just copy it over
	if (strlen(buf) && !strstr(buf,"N/A")) strcpy(capitalisation,buf); else strcpy(capitalisation,"DEFAULT");
    }
      
  // build the SQL string
  sprintf(query,"select symbol from stockprices where symbol = \"%s\" and date = \"%s\"",Sym,qDate);
  if (mysql_query(mysql,query)) { print_error(mysql, "02 Failed to query database");	}
  result=mysql_store_result(mysql);
  if (result==NULL) { print_error(mysql, "store_results failed"); } 
  // build and run the update or insert query
  if (mysql_num_rows(result)) 	// existing data, just update it
    sprintf(query,"update stockprices set day_open=%s,day_high=%s,day_low=%s,day_close=%s,volume=%s,day_change=%s,previous_close=%s \
      where symbol = \"%s\" and date = \"%s\"",day_open,day_high,day_low,day_close,volume,day_change,prev_close,Sym,qDate);
  else 	// new date, insert it
    sprintf(query,"insert into stockprices (symbol,date,day_open,day_high,day_low,day_close,volume,day_change,previous_close) \
      VALUES(\"%s\",\"%s\",%s,%s,%s,%s,%s,%s,%s)",Sym,qDate,day_open,day_high,day_low,day_close,volume,day_change,prev_close);
  // update the database
  if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update stockprices for today");
  if (DEBUG)printf("%s\n",query); //TEST
  // update stockinfo
  sprintf(query,"update stockinfo set exchange=\"%s\",capitalisation=%s,low_52weeks=%s,high_52weeks=%s,earnings=%s,dividend=%s,p_e_ratio=%s,avg_volume=%s \
    where symbol = \"%s\"",exchange,capitalisation,low_52weeks,high_52weeks,earnings,dividend,p_e_ratio,avg_volume,Sym);
  if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update stockinfo");
  if (DEBUG) printf("%s\n",query); //TEST
  // update the beancounter timestamp
  if (!DEBUG) mysql_query(mysql,"update beancounter set data_last_updated = NOW()");
  if (DEBUG) printf("%s\n",query); //TEST
  
  // clean up memory and exit after updates
  curl_easy_cleanup(curl);
  free(chunk.memory);
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
