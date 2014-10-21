// RTquote.c
/* get real-time quote for a symbol from Google 
 * 
 * Parms:  Sym
 * compile:  gcc -Wall -O2 -ffast-math RTquote.c -o RTquote `mysql_config --include --libs` `curl-config --libs`
 * 
 * <script>google.finance.renderRelativePerformance();</script>
 * <div class=g-unit>
 * <div id=market-data-div class="id-market-data-div nwp g-floatfix">
 * <div id=price-panel class="id-price-panel goog-inline-block">
 * <div>
 * <span class="pr">
 * <span id="ref_6550_l">77.32</span>
 */

#define	DEBUG	0
#define _XOPENSOURCE

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<curl/curl.h>
#include	<ctype.h>

MYSQL *mysql;
#include        "Includes/print_error.inc"
#include        "Includes/valid_sym.inc"


struct	MemStruct {
  char *memory;
  size_t size;
};

#include	"Includes/ParseData.inc"

int main (int argc, char *argv[]) {
  char	query[1024];
  char	Sym[16];
  char	Price[16];
  char	errbuf[CURL_ERROR_SIZE];
  char	*qURL="http://www.google.com/finance?q=";	// cat exchange:Sym like "NYSE:KO"
  char 	*saveptr;
  char	*Ticker;
  int	x,loop;
  CURL *curl;
  CURLcode	res=0;
  struct	MemStruct chunk;
  MYSQL_RES *result = 0;
  MYSQL_ROW row;
  
  // parse cli parms
  if (argc < 2) {
    printf("Usage:  %s Sym [Sym]...\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // allocate memory and initialize things
  if ((Ticker = calloc(1,(argc-1)*16)) == NULL) {
    fprintf(stderr,"malloc failed, aborting process\n");
    exit(EXIT_FAILURE);
  }    
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
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 0);

  #include "Includes/beancounter-conn.inc"
  // pair the symbols and exchange in a memory table
  for (loop=0;loop<argc-1;loop++) {
    memset(Sym,0,sizeof(Sym));
    for (x=0;x<strlen(argv[loop+1]);x++) { Sym[x] = toupper(argv[loop+1][x]); }
    if (valid_sym(Sym) == EXIT_FAILURE) continue;
    // get exchange for Sym
    memset(query,0,sizeof(query));
    sprintf(query,"select trim(exchange) from stockinfo where symbol = \"%s\"",Sym);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed"); 
    row=mysql_fetch_row(result);
    sprintf(&Ticker[loop*16],"%s:%s",row[0],Sym);
    if (DEBUG) printf("%s\n",&Ticker[loop*16]);
  }	// end For loop 1
  // finished with the database
  if (result) mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"
  
  // get the online data and display it
  for (loop=0;loop<argc-1;loop++) {  
    sprintf(query,"%s%s",qURL,(char *)&Ticker[loop*16]);
    curl_easy_setopt(curl, CURLOPT_URL, query);
    chunk.size = 0;
    if (DEBUG) printf("%s\n",(char *)&Ticker[loop*16]);
    if (!DEBUG) res=curl_easy_perform(curl);
    if (res) {	// 0 means no errors
      if (res != 56 && res != 28) {
	printf("01 Error %d Unable to access the internet\n",res);
	printf("%s\n",errbuf);
	exit(EXIT_FAILURE);
      } else {	// retry
	sleep(5);
	res=curl_easy_perform(curl);
	if (res) {
	  printf("02 Error %d Unable to access the internet after retry\n",res);
	  printf("%s\n",errbuf);
	  exit(EXIT_FAILURE);
	}
      }
    }
    // was any data returned?
    if (strstr(chunk.memory,"404 Not Found")) {     
      printf("%s not found\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      exit(EXIT_FAILURE);
    }
    // do the token parsing here
    if ((saveptr = strstr(chunk.memory,"<span class=\"pr\">")) == NULL) {
      printf("Error parsing data for %s, exiting\n",Sym);
      exit(EXIT_FAILURE);
    }
    saveptr += strlen("<span class=\"pr\">")+1;
    saveptr = strchr(saveptr,'>')+1;	// should now be pointing at the current price
    memset(Price,0,sizeof(Price));
    strncpy(Price,saveptr,(strchr(saveptr,'<'))-saveptr);
    printf("%s\t%8s\n",strchr(&Ticker[loop*16],':')+1,Price);
  }	// end For loop 2
  
  curl_easy_cleanup(curl);
  free(chunk.memory);
  free(Ticker);
  exit(EXIT_SUCCESS);
}
