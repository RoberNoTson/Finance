// update_stock.c
/* Update of today's (or most recent available) data to stockinfo, stockprices for one symbol
 * Sometimes Yahoo provides bad dates in their data; you can force a correction to the most recent business day 
 * by passing the [force] parm. 
 * This program should normally be run on the same business day, after market close.
 * NOTE: be cautious with "force" as data will be overwritten with no validity checks
 * 
 * Parms: Sym [force]
 * compile: gcc -Wall -O2 -ffast-math -o update_stock update_stock.c `mysql_config --include --libs` `curl-config --libs`
 */

// set DEBUG to 1 for testing, 0 for production
#define		DEBUG	0
#define		_XOPEN_SOURCE
#define		_XOPENSOURCE
#define		DAY_SECONDS     3600*24

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>
#include	<ctype.h>

MYSQL *mysql;
struct	MemStruct {
  char *memory;
  size_t size;
};
struct	MemStruct chunk;
time_t t,t2;
struct tm *TM;
struct tm *TM2;
#include        "Includes/print_error.inc"
#include        "Includes/valid_sym.inc"
#include        "Includes/ParseData.inc"


void	Usage(char *prog) {
  printf("Usage:  %s Sym [force]\n \
  \tUpdate the beancounter stockinfo and stockprice with today's (or most recent) data\n\tfor the given symbol.\n \
  \tSupplying the \"force\" parm forces the data to be updated,\n\tcoercing the Date to today, but only if the market is closed.\n\tUse \"force\" with caution!\n", prog);
  exit(EXIT_FAILURE);
}

int main(int argc, char * argv[]) {
  char	qDate[12];
  char	query[1024];
  char	qURL[1024];
  char	Sym[16];
  char	thisDate[16];
  char	buf[1024];
  char	day_open[32]="null";
  char	day_close[32]="null";
  char	day_high[32]="null";
  char	day_low[32]="null";
  char	day_change[32]="null";
  char	volume[32]="null";
  char	prev_close[32]="null";
  char	exchange[32]="null";
  char	capitalisation[32]="null";
  char	low_52weeks[32]="null";
  char	high_52weeks[32]="null";
  char	earnings[32]="null";
  char	dividend[32]="null";
  char	p_e_ratio[32]="null";
  char	avg_volume[32]="null";
  char	mail_msg[1024];
  char	*saveptr2;
  MYSQL_RES *result;
  MYSQL_ROW row;
  time_t t,t2;
  struct tm *TM = 0;
  struct tm *TM2 = 0;
  int num_rows,x,force=0;
  char	errbuf[CURL_ERROR_SIZE];
  CURL *curl;
  CURLcode	res;
  
  // parse cli parms
  if (argc == 1 || argc >3) Usage(argv[0]);
  if (argc >= 2) {
    // convert symbol parm to uppercase
    memset(Sym,0,sizeof(Sym));
    for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
    // verify symbol exists in stockinfo, else exit
    #include "Includes/beancounter-conn.inc"
    valid_sym(Sym);
  }	// end If argc >= 2
  // is it a valid parm?
  if (argc == 3) {
    if (strcasecmp(argv[2],"force")) Usage(argv[0]);
    // otherwise turn on the "force" flag to coerce invalid dates to this one
    force++;
  }
    
  // initialize the time structures with today
  t = t2 = time(NULL);
  TM2 = localtime(&t);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);
  // is this a business day?
  if (TM->tm_wday>0 && TM->tm_wday<6) {
    // yes, make sure it is 30 minutes past market close time on a business day
    if (TM->tm_hour<15 || (TM->tm_hour==15 && TM->tm_min<30)) {
	printf("No update for %s, market is still open today\n",Sym); 
	exit(EXIT_FAILURE);
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
  sprintf(query,"select last_bus_day from Investments.holidays where holiday = \"%s\"",qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  num_rows = mysql_num_rows(result);
  if (num_rows == 1) {	// holiday today, use last business day date
    if ((row=mysql_fetch_row(result))==NULL) {
      puts("oops - count() failed");
      exit(EXIT_FAILURE);
    }
    strcpy(qDate,row[0]);	// point to last_bus_day
    strptime(qDate,"%F",TM);	// ensure t and TM are concurrent with qDate
    t = mktime(TM);
  }
  mysql_free_result(result);

  // use YahooQuote to get today's data, if  local time > 15:30    
  // get date,close,open,prev_close,volume,change,low-high
  chunk.memory = calloc(1,1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) {
    fprintf(stderr,"curl init failed, aborting process\n");
    exit(EXIT_FAILURE);
  }    
  chunk.size=0;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=d1l1opvc1mxj1edra2w",Sym);
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
  res=curl_easy_perform(curl);
  if (res) {	// error, no data retrieved
	printf("Error %d Unable to access the internet\nRetrying %s...\n",res,Sym);
	sleep(5);
	res=curl_easy_perform(curl);
	if (res) {	// error, no data retrieved
	  printf("%s\n",errbuf);
	  print_error(mysql,"curl error, no data retrieved");
	}
  }
  // was any data returned?
  if (strstr(chunk.memory,"404 Not Found")) {     
    curl_easy_setopt(curl, CURLOPT_URL, qURL);
    printf("%s not found\n",Sym);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    #include "Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }

  // parse the supplied data
  memset(thisDate,0,sizeof(thisDate));
  memset(TM2,0,sizeof(struct tm));
  sscanf(chunk.memory,"%*[\"]%[0-9/NA]%*[\",]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9/NA]%*[,]%[0-9.-+/NA]%*[\",]%[0-9./NA]%*[ -]%[0-9./NA]%*[\",]%[A-Za-z/]%*[\",]%[0-9.BMTK/NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9/NA]%*[\",]%[0-9./NA]%*[ -]%[0-9./NA]%*s",
      thisDate,day_close,day_open,prev_close,volume,day_change,day_low,day_high,exchange, capitalisation,earnings,dividend,p_e_ratio,avg_volume,low_52weeks,high_52weeks);
  // is it valid?
  if (!strcmp(thisDate,"\"N/A\"")) {	// bad symbol, deactivate it
    printf("Deactivated bad symbol %s\n",Sym);
    sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
    if (!DEBUG) mysql_query(mysql,query);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    #include "Includes/mysql-disconn.inc"
    // email notification
    sprintf(mail_msg,"echo %s deactivated |mailx -s \"%s\" mark_roberson@tx.rr.com",Sym,Sym);
    system(mail_msg);   
    exit(EXIT_FAILURE);
  }
  // verify the retrieved date is valid, else force it to match qDate
  memset(TM2,0,sizeof(struct tm));
  // adjust the format to match ISO standard
  if (!strptime(thisDate,"%m/%d/%Y",TM2))  {
    strftime(thisDate,sizeof(thisDate),"%F",TM2);
    if (force) {
	  printf("Overriding pricing date to %s from %s for %s\n",qDate,thisDate,Sym);
    } else {	// bad date supplied, "force" not enabled, just quit now.
	  sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
	  if (!DEBUG) mysql_query(mysql,query);
	  printf("Deactivated bad symbol %s with invalid date\n",Sym);
	  curl_easy_cleanup(curl);
	  free(chunk.memory);
	  #include "Includes/mysql-disconn.inc"
	  exit(EXIT_FAILURE);
    }
  }
  if (strncmp(qDate,thisDate,10)) {
    // is the retrieved date in the future?
    TM2->tm_hour = 1;
    t2 = mktime(TM2);
    if (t2 > t) {	// future date?
      // yes, skip it unless "force" is enabled
	if (force) {
	  printf("Overriding pricing date to %s from %s for %s\n",qDate,thisDate,Sym);
	} else {	// bad date supplied, "force" not enabled, just quit now.
	  printf("Skipping %s on %s invalid future date %s\n",Sym,qDate,thisDate);
	  curl_easy_cleanup(curl);
	  free(chunk.memory);
	  #include "Includes/mysql-disconn.inc"
	  exit(EXIT_FAILURE);
	}
    } else if (t2 < (t-(7*DAY_SECONDS))) {
      // is it over a week in the past?
      // yes, skip it
	sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
	if (!DEBUG) mysql_query(mysql,query);
	printf("Deactivated symbol %s with old date %s\n",Sym,thisDate);
	curl_easy_cleanup(curl);
	free(chunk.memory);
	#include "Includes/mysql-disconn.inc"
	exit(EXIT_FAILURE);
    }
  }	// end If compare qDate,thisDate

  // start main processing
  // split out today's data
  if (!strcmp(day_close,"N/A")) strcpy(day_close,"DEFAULT");
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
  strcpy(buf,capitalisation); // save capitalisation
  // convert capitalisation format to millions
  if ((saveptr2=strchr(buf,'B'))!=NULL) {     // convert Billions to Millions
      memset(saveptr2,0,1);
      sprintf(capitalisation,"%.4f",strtod(buf,NULL)*1000);
  } else if ((saveptr2=strchr(buf,'M'))!=NULL) {      // already in Millions
      memset(saveptr2,0,1);
      strcpy(capitalisation,buf);
  } else if ((saveptr2=strchr(buf,'T'))!=NULL) {      // convert Trillions to Millions
      memset(saveptr2,0,1);
      sprintf(capitalisation,"%.4f",strtod(buf,NULL)*1000000);
  } else if ((saveptr2=strchr(buf,'K'))!=NULL) {      // convert Thousands to Millions
      memset(saveptr2,0,1);
      sprintf(capitalisation,"%.4f",strtod(buf,NULL)*0.001);
  } else {    // just copy it over
      if (strlen(buf) && !strstr(buf,"N/A")) strcpy(capitalisation,buf); 
      else strcpy(capitalisation,"DEFAULT");
  }	// end convert capitalisation format to millions

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
  mysql_free_result(result);
  // update the database
  if (!DEBUG) if (mysql_query(mysql,query))  print_error(mysql, "Failed to update stockprices for today");
  if (DEBUG) printf("%s\n",query);
  // update stockinfo
  sprintf(query,"update stockinfo set exchange=\"%s\",capitalisation=%s,low_52weeks=%s,high_52weeks=%s,earnings=%s,dividend=%s,p_e_ratio=%s,avg_volume=%s \
    where symbol = \"%s\"",exchange,capitalisation,low_52weeks,high_52weeks,earnings,dividend,p_e_ratio,avg_volume,Sym);
  if (!DEBUG)  if (mysql_query(mysql,query))  print_error(mysql, "Failed to update stockinfo");
  if (DEBUG) printf("%s\n",query);
  // update the beancounter timestamp
  mysql_query(mysql,"update beancounter set data_last_updated = NOW()");
  
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  curl_easy_cleanup(curl);
  free(chunk.memory);
  exit(EXIT_SUCCESS);
}
