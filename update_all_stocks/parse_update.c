/* parse_update.c
 * part of update_all_stocks
 */
#include	"update_all_stocks.h"

int	parse_update(char *Sym) {
  char	qURL[1024];
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
  char	query[1024];
  char	buf[1024];
  char	thisDate[16];
  char	mail_msg[1024];
  char	*saveptr2;

  MYSQL	*mysql;
  MYSQL_ROW	row;
  MYSQL_RES *result;
  char	errbuf[CURL_ERROR_SIZE];
  CURL *curl;
  CURLcode	res;

  // use YahooQuote to get today's data if after market-close
  printf("Updating data for %s\n",Sym);
  // get date,close,open,prev_close,volume,change,low-high
    curl=curl_easy_init();
    if(!curl) {
      fprintf(stderr,"curl init failed, aborting process\n");
      exit(EXIT_FAILURE);
    }    
    chunk.size=0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);	// must be > CURLOPT_CONNECTTIMEOUT
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);	// boolean value 0 or 1, use 1 for multi-threading
    sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=d1l1opvc1mxj1edra2w",Sym);
    curl_easy_setopt(curl, CURLOPT_URL, qURL);
    res=curl_easy_perform(curl);
    if (res) {	// error, no data retrieved
	printf("Error %d Unable to access the internet\nRetrying %s...\n",res,Sym);
	sleep(10);
	res=curl_easy_perform(curl);
	if (res) {	// error, no data retrieved
	  printf("%s\n",errbuf);
	  print_error(mysql,"curl error, no data retrieved");
	  printf("for %s\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      exit(EXIT_FAILURE);
	}
    }
    // was any data returned?
    if (strstr(chunk.memory,"404 Not Found")) {
      printf("%s not found\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      exit(EXIT_FAILURE);
    }

  #include      "/Finance/bin/C/src/Includes/beancounter-conn.inc"
  // parse the supplied data
  memset(thisDate,0,sizeof(thisDate));
  sscanf(chunk.memory,"%*[\"]%[0-9/NA]%*[\",]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9/NA]%*[,]%[0-9.-+/NA]%*[\",]%[0-9./NA]%*[ -]%[0-9./NA]%*[\",]%[A-Za-z/]%*[\",]%[0-9.BMTK/NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9/NA]%*[\",]%[0-9./NA]%*[ -]%[0-9./NA]%*s",
      thisDate,day_close,day_open,prev_close,volume,day_change,day_low,day_high,exchange, capitalisation,earnings,dividend,p_e_ratio,avg_volume,low_52weeks,high_52weeks);
  // is it valid?
  if (!strcmp(thisDate,"\"N/A\"")) {	// bad symbol, deactivate it
    sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
    if (!DEBUG) mysql_query(mysql,query);
    printf("Deactivated bad symbol %s\n",Sym);
    curl_easy_cleanup(curl);
    free(chunk.memory);
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
	  #include "../Includes/mysql-disconn.inc"
	  exit(EXIT_FAILURE);
	}
    } else if (t2 < (t-(8*DAY_SECONDS))) {
      // deactivate it if over a week since the last update
	sprintf(query,"update stockinfo set active=false where symbol = \"%s\"",Sym);
	if (!DEBUG) mysql_query(mysql,query);
	printf("Deactivated symbol %s with old date %s\n",Sym,thisDate);
	curl_easy_cleanup(curl);
	free(chunk.memory);
	#include "../Includes/mysql-disconn.inc"
    // email notification
      sprintf(mail_msg,"echo %s deactivated |mailx -s \"%s\" mark_roberson@tx.rr.com",Sym,Sym);
      system(mail_msg);   
	exit(EXIT_FAILURE);
    }
  }	// end If compare qDate,thisDate

  // parse curl data
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

    // update stockprices
    sprintf(query,"insert into stockprices (symbol,date,\
    day_open,day_high,day_low,day_close,volume,day_change,previous_close) \
    VALUES(\"%s\",\"%s\",\
    %s,%s,%s,%s,%s,%s,%s) \
    ON DUPLICATE KEY UPDATE \
    day_open=VALUES(day_open),day_high=VALUES(day_high),day_low=VALUES(day_low),\
    day_close=VALUES(day_close),volume=VALUES(volume),day_change=VALUES(day_change),\
    previous_close=VALUES(previous_close)",
    Sym,qDate,day_open,day_high,day_low,day_close,volume,day_change,prev_close);
    if (!DEBUG) if (mysql_query(mysql,query))  print_error(mysql, "Failed to update stockprices for today");
    if (DEBUG) printf("%s\n",query);
    // update stockinfo
    sprintf(query,"update stockinfo set exchange=\"%s\",capitalisation=%s,low_52weeks=%s,\
    high_52weeks=%s,earnings=%s,dividend=%s,p_e_ratio=%s,avg_volume=%s \
    where symbol = \"%s\"",
    exchange,capitalisation,low_52weeks,high_52weeks,earnings,dividend,p_e_ratio,avg_volume,Sym);
    if (!DEBUG)  if (mysql_query(mysql,query))  print_error(mysql, "Failed to update stockinfo");
    if (DEBUG) printf("%s\n",query);
    // update the beancounter timestamp
    if (!debug) mysql_query(mysql,"update beancounter set data_last_updated = NOW()");
    if (debug) puts("update beancounter set data_last_updated = NOW()");
    curl_easy_cleanup(curl);
  // finished with the database
  #include "../Includes/mysql-disconn.inc"

  return(EXIT_SUCCESS);
}
