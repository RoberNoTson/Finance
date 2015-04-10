// add_stock.c
/* Add a new symbol to beancounter.stockinfo and Investments.portfolio
 * 
 * Parms: Sym
 * compile:  gcc -Wall -O2 -ffast-math add_stock.c -o add_stock `mysql_config --include --libs` `curl-config --libs`
 */

#define		DEBUG	0
#define		_XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>
#include	<ctype.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"

struct	MemStruct {
  char *memory;
  size_t size;
};

#include	"Includes/ParseData.inc"

int	main (int argc, char *argv[]) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  char	Sym[32];
  char	exchange[16];
  char	name[1024];
  char	query[1024];
  char      errbuf[CURL_ERROR_SIZE];
  char	*saveptr;
  int	x;
  CURL *curl;
  CURLcode	res;
  struct	MemStruct chunk;
  
  // parse cli parms
  if (argc != 2) {
    printf("Usage:  %s Sym\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  // convert symbol parm to uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  // verify symbol exists in stockinfo, else exit
  #include "Includes/beancounter-conn.inc"
  sprintf(query,"select symbol,active from stockinfo where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  if (!mysql_num_rows(result)) {
    // no rows returned, add it to stockinfo
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
    sprintf(query,"http://download.finance.yahoo.com/d/quotes.csv?f=snx&e=.csv&s=%s",Sym);
    curl_easy_setopt(curl, CURLOPT_URL, query);
    res=curl_easy_perform(curl);
    if (res) {	// 0 means no errors
      sleep(5); // wait and retry
      res=curl_easy_perform(curl);
      if (res) {
	printf("Error %d Unable to access the internet\n",res);
	printf("%s\n",errbuf);
	print_error(mysql,"curl error, no data retrieved");
      }
    }
    // was any data returned?
    if (strstr(chunk.memory,"404 Not Found")) {     
      printf("%s not found\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      #include "Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
    // got data, parse to verify it is valid
    saveptr=strtok(chunk.memory,",");	//skip symbol
    saveptr=strtok(NULL ,"\"");	//name
    strcpy(name,saveptr);
    if ((strcmp(name,"\"N/A\"")==0)) {
      printf("%s not found at Yahoo\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      #include "Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }      
    saveptr=strtok(NULL ,"\"");	//exchange
    saveptr=strtok(NULL ,"\"");	//exchange
    strcpy(exchange,saveptr);
    if ((strcmp(exchange,"\"N/A\"")==0)) {
      printf("%s not found at Yahoo\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      #include "Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
    }
    // good data downloaded, update the database
    sprintf(query,"insert into stockinfo (symbol,name,exchange) values(\"%s\",\"%s\",\"%s\")",Sym,name,exchange);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if (mysql_query(mysql,query)) print_error(mysql, "Failed to insert new symbol");
    curl_easy_cleanup(curl);
    free(chunk.memory);
    printf("%s added to database\n",Sym);
  } else {
    // already in stockinfo, is it active?
    row=mysql_fetch_row(result);
    if(row == NULL) print_error(mysql,"02 Fatal error - Failed to query database in add_stock"); 
    if (!strcmp(row[1],"0")) {
     printf("%s already exists in the database but is inactive - do you wish to activate it? [y/n] ",Sym); 
     query[0] = getchar();
      if (strncmp(query,"y",1)) {
	puts("No changes made");
	exit(EXIT_FAILURE);
      }
      sprintf(query,"update stockinfo set active = true where symbol = \"%s\"",Sym);
      if (mysql_query(mysql,query)) print_error(mysql, "Fatal error - Failed to activate new symbol");
      printf("%s activated\n",Sym);

    } else {
      printf("%s already present and active in the database, no action taken\n",Sym);
      mysql_free_result(result);
      #include "Includes/mysql-disconn.inc"
      exit(EXIT_SUCCESS);
    }
  }
    sprintf(query,"select max(date) from stockprices where symbol = \"%s\"",Sym);
    if (mysql_query(mysql,query)) print_error(mysql, "01 Failed to query database");
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
    if (!mysql_num_rows(result)) {
    // no rows returned, add it?
      printf("No stockprice data found for %s, run \"backpop\" on it\n",Sym);
    } else {
      row=mysql_fetch_row(result);
      if(row == NULL) print_error(mysql,"02 Fatal error - Failed to query database in add_stock"); 
      printf("Last update for %s was %s\nConsider if you need to run \"backpop\"\n",Sym,row[0]);
    }
  mysql_free_result(result);
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
