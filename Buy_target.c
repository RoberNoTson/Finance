/* Buy_target.c
 * monitor real-time price for a given symbol, notify via email and/or popup when target buy price is reached.
 * Parms: Sym Price(as 12.34)
 * 
 * compile: gcc -Wall -O2 -ffast-math -o Buy_target Buy_target.c `mysql_config --include --libs` `curl-config --libs`
 */

// set DEBUG to 1 for testing, 0 for production
#define         DEBUG   0
#define		EMAIL 1
#define		VERBOSE 1
#define		SLEEP_INTERVAL 120
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
  char	SymExch[32];
  char	mail_msg[1024];
  char	errbuf[CURL_ERROR_SIZE];
  char	market_open[12]=MARKET_OPEN;
  char	market_close[12]=MARKET_CLOSE;
  char 	*saveptr;
  int	x;
  int	verbose=VERBOSE;
  int	email=EMAIL;
  int	interval=SLEEP_INTERVAL;
  float	TargetPrice=FLT_MAX;
  float	cur_price=0.0;
  CURL *curl;
  CURLcode	res=0;
  MYSQL_RES *result;
  MYSQL_ROW row;
  time_t t;
  struct tm *TM=0;

  // parse cli parms
  if (argc < 3) Usage(argv[0]);
  // set up variables
  memset(query,0,sizeof(query));
  memset(SymExch,0,sizeof(SymExch));
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]);
  errno=0;
  if ((TargetPrice = strtof(argv[2],NULL)) == 0) Usage(argv[0]);
  if (errno == ERANGE) { puts("Error converting price"); Usage(argv[0]); }

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
  sprintf(SymExch,"%s:%s",row[0],Sym);
  if (DEBUG) printf("%s\n",SymExch);
  // finished with the database
  mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"
  
  sprintf(query,"http://www.google.com/finance?q=%s",SymExch);
  curl_easy_setopt(curl, CURLOPT_URL, query);
  
  // Loop while price is less than target
  if (DEBUG) printf("Monitoring price for %s\n",Sym);
  do {
    // check the time
    t = time(NULL);
    if ((TM = localtime(&t)) == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
    if (DEBUG) printf("Close: %lu\tTM %d\n",strtoul(market_close,&saveptr,10),TM->tm_hour);
    if (TM->tm_hour>strtoul(market_close,&saveptr,10) || 
      (TM->tm_hour==strtoul(market_close,&saveptr,10) && TM->tm_min>(strtoul(saveptr+1,NULL,10)+(interval/60)))) {
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
	sleep(interval);
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
	sleep(interval);
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
      sleep(interval);
      continue;
    }
    // do the token parsing here
    if ((saveptr = strstr(chunk.memory,"<span class=\"pr\">")) == NULL) {
      if (DEBUG) printf("Error parsing data for %s, retrying\n",Sym);
      sleep(interval);
      continue;
    }
    saveptr += strlen("<span class=\"pr\">")+1;
    saveptr = strchr(saveptr,'>')+1;	// should now be pointing at the current price
    memset(Price,0,sizeof(Price));
    strncpy(Price,saveptr,(strchr(saveptr,'<'))-saveptr);
    errno=0;
    if ((cur_price = strtof(Price,NULL)) == 0) break;
    if (errno == ERANGE) break;
    if (DEBUG) printf("cur_price: %.2f Target: %.2f\n",cur_price,TargetPrice);
    if (DEBUG) printf("Pausing for %d seconds\n",interval);
    if (cur_price <= TargetPrice) break;
    sleep(interval);
  } while(cur_price > TargetPrice);
  // end While loop

  // process the output message
  sprintf(query,"Buy %s at %.2f target price",Sym,cur_price);
  if (strlen(query)) {
    if (verbose) printf("%s\n",query);
    if (email) {
	  sprintf(mail_msg,"echo %s |mailx -s \"%s\" mark_roberson@tx.rr.com",query,query);
	  system(mail_msg);
    }
  }
  curl_easy_cleanup(curl);
  free(chunk.memory);
  exit(EXIT_SUCCESS);
}
