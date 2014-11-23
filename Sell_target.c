/* Sell_target.c
 * monitor real-time price for a given symbol, notify via email and/or popup when target long price is reached.
 * Parms: Sym Price(as 12.34)
 * 
 * compile: gcc -Wall -O2 -ffast-math -o Sell_target Sell_target.c -liniparser `mysql_config --include --libs` `curl-config --libs`
 */

// set DEBUG to 1 for testing, 0 for production
#define         DEBUG   0
#define		INIFILE "notify.list"
#define		SLEEP_INTERVAL 90
#define		MARKET_CLOSE	"15:00"
#define		MARKET_OPEN	"8:30"
#define         _XOPEN_SOURCE
#define         _XOPENSOURCE

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <unistd.h>
#include        <curl/curl.h>
#include        <ctype.h>
#include	<iniparser.h>

MYSQL *mysql;
struct  MemStruct {
  char *memory;
  size_t size;
};
struct  MemStruct chunk;

#include        "Includes/print_error.inc"
#include        "Includes/valid_sym.inc"
#include        "Includes/ParseData.inc"
 
void    Usage(char *prog) {
  printf("Usage:  %s Sym Price(format as 43.21)\n", prog);
  exit(EXIT_FAILURE);
}

int main(int argc, char * argv[]) {
  char  query[1024];
  char	Sym[16];
  char	Price[16];
  char	Ticker[32];
  char	mail_msg[1024];
  char	ini_file[1024];
  char	errbuf[CURL_ERROR_SIZE];
  char	market_open[12]=MARKET_OPEN;
  char	market_close[12]=MARKET_CLOSE;
  char 	*saveptr;
  int	x;
  float	TargetPrice=FLT_MAX;
  float	cur_price=0.0;
  CURL *curl;
  CURLcode	res=0;
  MYSQL_RES *result;
  MYSQL_ROW row;
  time_t t;
  struct tm *TM=0;
  dictionary *d;
  
  // parse cli parms
  if (argc < 3) Usage(argv[0]);
  // set up variables
  memset(query,0,sizeof(query));
  memset(Ticker,0,sizeof(Ticker));
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
  TargetPrice = strtof(argv[2],NULL);

  // initialize CURL
  if ((chunk.memory = calloc(1,1)) == NULL) {
    fprintf(stderr,"calloc failed, aborting process\n");
    exit(EXIT_FAILURE);
  }    
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) {
    fprintf(stderr,"curl init failed, aborting process\n");
    exit(EXIT_FAILURE);
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  #include "Includes/beancounter-conn.inc"
  valid_sym(Sym);

  // get exchange for Sym
  sprintf(query,"select trim(exchange) from stockinfo where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed"); 
  row=mysql_fetch_row(result);
  sprintf(Ticker,"%s:%s",row[0],Sym);
  if (DEBUG) printf("%s\n",Ticker);
  // finished with the database
  mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"
  
  sprintf(query,"http://www.google.com/finance?q=%s",Ticker);
  curl_easy_setopt(curl, CURLOPT_URL, query);
  
  // Loop while price is less than target
  if (DEBUG) printf("Monitoring price for %s\n",Sym);
  do {
    // check the time
    t = time(NULL);
    if ((TM = localtime(&t)) == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
    if (DEBUG) printf("Close: %lu\tTM %d\n",strtoul(market_close,&saveptr,10),TM->tm_hour);
    if (TM->tm_hour>strtoul(market_close,&saveptr,10) || 
      (TM->tm_hour==strtoul(market_close,&saveptr,10) && TM->tm_min>(strtoul(saveptr+1,NULL,10)+(SLEEP_INTERVAL/60)))) {
      // market is closed for the day
      printf("Stock market is closed for the day (after %s)\n",market_close);
      exit(EXIT_FAILURE);
    }
    if (DEBUG) printf("Open: %lu\n",strtoul(market_open,&saveptr,10));
    if (TM->tm_hour<strtoul(market_open,&saveptr,10) || 
      (TM->tm_hour==strtoul(market_open,&saveptr,10) && TM->tm_min<atoi(saveptr+1))) {
      printf("Stock market not open yet, sleeping until %s\n",market_open);
      sleep((strtoul(market_open,&saveptr,10)-TM->tm_hour)*3600);
      while (atoi(saveptr+1)>TM->tm_min) {
	sleep(SLEEP_INTERVAL);
	t = time(NULL);
	if ((TM = localtime(&t)) == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
	x=strtoul(market_open,&saveptr,10);
      }
    }
    // download current price
    chunk.size = 0;
    res=curl_easy_perform(curl);
    if (res) {	// 0 means no errors
      if (res != 56 && res != 28) {
	printf("01 Error %d Unable to access the internet\n",res);
	printf("%s\n",errbuf);
	exit(EXIT_FAILURE);
      } else {	// retry
	if (DEBUG) puts("CURL error connecting to the internet, retrying");
	sleep(SLEEP_INTERVAL);
	continue;
      }
    }
    // was any data returned?
    if (strstr(chunk.memory,"404 Not Found")) {     
      printf("%s not found\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      exit(EXIT_FAILURE);
    }
    if (chunk.size == 0) {
      if (DEBUG) printf("Error receiving data for %s, retrying\n",Sym);
      sleep(SLEEP_INTERVAL);
      continue;
    }
    // do the token parsing here
    if ((saveptr = strstr(chunk.memory,"<span class=\"pr\">")) == NULL) {
      if (DEBUG) printf("Error parsing data for %s, retrying\n",Sym);
      sleep(SLEEP_INTERVAL);
      continue;
    }
    saveptr += strlen("<span class=\"pr\">")+1;
    saveptr = strchr(saveptr,'>')+1;	// should now be pointing at the current price
    memset(Price,0,sizeof(Price));
    strncpy(Price,saveptr,(strchr(saveptr,'<'))-saveptr);
    cur_price = strtof(Price,NULL);
    if (DEBUG) printf("cur_price: %.2f Target: %.2f\n",cur_price,TargetPrice);
    if (DEBUG) printf("Pausing for %d seconds\n",SLEEP_INTERVAL);
    if (cur_price >= TargetPrice) break;
    sleep(SLEEP_INTERVAL);
  } while(cur_price < TargetPrice);
  // end While loop
  curl_easy_cleanup(curl);
  free(chunk.memory);

  // process the output message
  sprintf(ini_file, "%s/%s", getenv("HOME"), INIFILE);
  d = iniparser_load(ini_file);
  if (d==NULL) {
    puts("INI file not opened, aborting");
    exit(EXIT_FAILURE);
  }
  sprintf(query,"Sell %s at %.2f target price",Sym,cur_price);
  printf("%s\n",query);
  if (iniparser_find_entry(d, "Dest:email")) {
    sprintf(mail_msg,"echo %s |mailx -s \"%s\" %s",query,query, iniparser_getstring(d, "Dest:email", "null"));
    system(mail_msg);
  }
  if (iniparser_find_entry(d, "Dest:sms")) {
    sprintf(mail_msg,"echo %s |mailx -s \"%s\" %s",query,query, iniparser_getstring(d, "Dest:sms", "null"));
    system(mail_msg);
  }
  iniparser_freedict(d);
  exit(EXIT_SUCCESS);
}
