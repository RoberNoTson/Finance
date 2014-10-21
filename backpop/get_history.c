/* get_history.c
 * part of backpop
 */
#include	"./backpop.h"

int	get_history(char *Sym, char *qDate) {
  char	*histURL="http://table.finance.yahoo.com/table.csv?a=$a&b=$b&c=$c&d=$d&e=$e&f=$f&s=";
  char	*histURL2="&y=0&g=d&ignore=.csv";
  char	prev_date[12];
  char	query[1024];
  char	qURL[1024];
  char	thisDate[12];
  char	day_open[32]="DEFAULT";
  char	day_close[32]="DEFAULT";
  char	day_high[32]="DEFAULT";
  char	day_low[32]="DEFAULT";
  char	volume[32]="DEFAULT";
  char	prev_close[32]="DEFAULT";
//  int	updated=0;
  char 	*saveptr;
  
  // initialize variables
  printf("Backfilling data for %s since %s\n",Sym,qDate);
  chunk.memory = calloc(1,1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) {
    fprintf(stderr,"curl init failed, aborting process\n");
    exit(EXIT_FAILURE);
  }    
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  sprintf(qURL,"%s%s%s",histURL,Sym,histURL2);
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
  res=curl_easy_perform(curl);
  if (res) {	// error, no data retrieved
	if (DEBUG) printf("Error %d Unable to access the internet\nRetrying %s...\n",res,Sym);
	sleep(5);
	res=curl_easy_perform(curl);
	if (res) {	// error, no data retrieved
	  printf("%s\n",errbuf);
	  print_error(mysql,"curl error, no data retrieved");
	}
  }
  // was any data returned?
  if (strstr(chunk.memory,"404 Not Found")) { 
    printf("%s history data not found online, \"404\" error\n",Sym);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    #include "../Includes/mysql-disconn.inc"
    exit(EXIT_FAILURE);
  }
  
  // skip the first line of headers
  saveptr = strchr(chunk.memory,'\n');
  memset(prev_date,0,sizeof(prev_date));
  updated = 0;
  
  // loop until out of data or reached qDate
  do {
    if (saveptr >= chunk.memory+chunk.size) break;
    sscanf(saveptr+1,"%[-0-9/NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,]%[0-9./NA]%*[,0-9/NA]\n",thisDate,day_open,day_high,day_low,day_close,volume);
    if (DEBUG) printf("%s,%s,%s,%s,%s,%s\n",thisDate,day_open,day_high,day_low,day_close,volume);

    // validate the date
    if (!strcmp(thisDate,"N/A")) {
      printf("%s has no history data online\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
    // check that we haven't passed the prev_date
    strptime(thisDate,"%F",TM2);
    t2=mktime(TM2);
    if (t > t2) break;
    // validate prices
    if (strstr(day_open,"N/A")) strcpy(day_open,"DEFAULT");
    if (strstr(day_high,"N/A")) strcpy(day_high,"DEFAULT");
    if (strstr(day_low,"N/A")) strcpy(day_low,"DEFAULT");
    if (strstr(day_close,"N/A")) strcpy(day_close,"DEFAULT");
    if (strstr(volume,"N/A")) strcpy(volume,"DEFAULT");
    
    // determine if this is an insert or update
    sprintf(query,"select symbol from stockprices where symbol = \"%s\" and date = \"%s\"",Sym,thisDate);
    if (mysql_query(mysql,query)) { print_error(mysql, "02 Failed to query database");	}
    result=mysql_store_result(mysql);
    if (result==NULL) { print_error(mysql, "store_results failed"); } 
    // build and run the update or insert query
    if (mysql_num_rows(result)) 	// existing data, just update it
      sprintf(query,"update stockprices set day_open=%s,day_high=%s,day_low=%s,day_close=%s,volume=%s,day_change=DEFAULT,previous_close=DEFAULT \
      where symbol = \"%s\" and date = \"%s\"",day_open,day_high,day_low,day_close,volume,Sym,thisDate);
    else 	// new date, insert it
      sprintf(query,"insert into stockprices (symbol,date,day_open,day_high,day_low,day_close,volume) \
      VALUES(\"%s\",\"%s\",%s,%s,%s,%s,%s)",Sym,thisDate,day_open,day_high,day_low,day_close,volume);
    if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
    if (DEBUG) printf("%s\n",query);
    // update previous close and day_change for next higher row
    if (strlen(prev_date)) {
      if (strcmp(prev_close,"null")) {
	sprintf(query,"update stockprices set previous_close = \"%s\" where symbol = \"%s\" and date = \"%s\"",day_close,Sym,prev_date);
	if (!DEBUG)      if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
	if (DEBUG)printf("%s\n",query);	//TEST
	sprintf(query,"update stockprices set day_change = \"%.2f\" where symbol = \"%s\" and date = \"%s\"",strtof(prev_close,NULL)-strtof(day_close,NULL),Sym,prev_date);
	if (!DEBUG)      if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
	if (DEBUG)printf("%s\n",query);	//TEST
      }
    }
    updated++;
    if (!strcmp(qDate,thisDate)) break;	// reached the oldest requested date, quit
    strcpy(prev_date,thisDate);
    strcpy(prev_close,day_close);
  } while ((saveptr=strchr(saveptr+1,'\n')) != NULL);
  // end While loop
  
  // update the previous_close and day_change for oldest member, if data exists in the database
  if (!updated) return EXIT_FAILURE;
    
  sprintf(query,"select day_close from stockprices where symbol = \"%s\" and date < \"%s\" \
    order by date desc limit 1",Sym,thisDate);
  if (mysql_query(mysql,query))  print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if (result==NULL) print_error(mysql, "store_results failed");
  
  // update if results exist, else set values to NULL
  if (mysql_num_rows(result)) {
    row=mysql_fetch_row(result);
    if (strlen(thisDate)) {
	sprintf(query,"update stockprices set previous_close = \"%s\" where symbol = \"%s\" and date = \"%s\"",row[0],Sym,thisDate);
	if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
	if (DEBUG)printf("%s\n",query); //TEST
	sprintf(query,"update stockprices set day_change = \"%.2f\" where symbol = \"%s\" and date = \"%s\"",strtof(day_close,NULL)-strtof(row[0],NULL),Sym,thisDate);
	if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
	if (DEBUG)printf("%s\n",query); //TEST
    }
  } else {
    sprintf(query,"update stockprices set previous_close = \"null\" where symbol = \"%s\" and date = \"%s\"",Sym,thisDate);
    if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
    if (DEBUG)printf("%s\n",query); //TEST
    sprintf(query,"update stockprices set day_change = \"null\" where symbol = \"%s\" and date = \"%s\"",Sym,thisDate);
    if (!DEBUG)    if (mysql_query(mysql,query))  print_error(mysql, "Failed to update database");
    if (DEBUG)printf("%s\n",query); //TEST
  }
  // update the beancounter timestamp
  if (!DEBUG)  if (updated) mysql_query(mysql,"update beancounter set data_last_updated = NOW()");

  // clean up memory
  curl_easy_cleanup(curl);
  free(chunk.memory);
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  if (DEBUG) printf("Updates complete\n");
  exit(EXIT_SUCCESS);
}
