// add_missing_syms.c - weekly (or on demand) add new symbols to beancounter database
/* 1. pull all symbols from NASDAQ website, compare to beancounter.stockinfo
 * 2. add new symbols to beancounter.stockinfo
 * 3. get pricing history for prior 12 months, or as much as exists into beancounter.stockprices
 * 4. create an updated copy of "/Finance/bin/all-local-symbols"
 * 
 * Parms:  none
 * compile:  gcc -Wall -O2 -ffast-math -o add_missing_syms add_missing_syms.c `mysql_config --include --libs` `curl-config --libs`
 */

#define		DEBUG 0
#define		MINVOLUME "80000"
#define		MINPRICE "14"
#define		MAXPRICE "250"

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>
#include        <ctype.h>

MYSQL *mysql;
#include        "Includes/print_error.inc"

char	*qNASDAQ="http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=nasdaq&render=download";
char	*qNYSE="http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=nyse&render=download";
char	*qAMEX="http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=amex&render=download";
char	errbuf[CURL_ERROR_SIZE];
CURL *curl;
CURLcode	res;
struct	MemStruct {
  char *memory;
  size_t size;
};
struct	MemStruct chunk;
struct	MemStruct info;

#include	"Includes/ParseData.inc"

int GetData(char *qURL) {
  char 	*saveptr=0;
  char	*infoptr;
  char	*Symptr;
  char	*Exchptr=0;
  char	Ticker[128];
  char	exchange[16];
  char	name[1024];
  char	query[1024];
  char	theYear[11];
  char	*qURL1="http://download.finance.yahoo.com/d/quotes.csv?s=";
  char	*qURL2="&e=.csv&f=snxj1wedra2";
  MYSQL_RES *result;
  MYSQL_ROW row;
  struct tm *TM;
  time_t t;
  
  t = time(NULL);
  TM = localtime(&t);
  TM->tm_year--;
  strftime(theYear, sizeof(theYear), "%F", TM);
  chunk.size = 0;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
//  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
//  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.6) Gecko/20070802 SeaMonkey/1.1.4");
  res=curl_easy_perform(curl);
  if (res) {
    sleep(5);
    res=curl_easy_perform(curl);
    if (res) {
      printf("Error %d Unable to access the internet\n",res);
      printf("%s\n",errbuf);
      return(res);
    }
  }	// end If !res
  strtok_r(chunk.memory,",",&saveptr);	// skip the first line of headers
  strtok_r(NULL ,"\012",&saveptr);
  
  while ((Symptr=strtok_r(NULL,",",&saveptr)) != NULL) {
	memset(query,0,sizeof(query));
	memset(Ticker,0,sizeof(Ticker));
	sscanf(Symptr+1, "%s", Ticker); // skip the initial double-quote and any blank space
//if (DEBUG) printf("%s\n",Symptr);
	strtok_r(NULL ,"\n",&saveptr);	// point to the next line
	if (strchr(Ticker,'"')) memset(strchr(Ticker,'"'),0,1);	// remove the trailing double-quote
	
  // Process the ticker symbol
	// skip tickers with special characters
	if (strchr(Ticker,'^')) continue;
	if (strchr(Ticker,'/')) continue;
    // see if it already exists in the db
	sprintf(query,"select count(symbol) from stockinfo where symbol=\"%s\"",Ticker);
	if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query database"); }
	result=mysql_store_result(mysql);
	if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
	if ((row=mysql_fetch_row(result))==NULL) {
	  puts("oops - count() failed");
	  mysql_free_result(result);
	  exit(EXIT_FAILURE);
	}
	if ((strcmp(row[0],"0"))>0) {
//	   if (DEBUG) puts("found in stockinfo");
	    mysql_free_result(result);
	   continue;
	}
	mysql_free_result(result);
    // new ticker, add it to stockinfo
	memset(query,0,sizeof(query));
	sprintf(query,"%s%s%s",qURL1,Ticker,qURL2);
	info.size = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&info);
	curl_easy_setopt(curl, CURLOPT_URL, query);
	res=curl_easy_perform(curl);
	if (res) {
	  sleep(5);
	  res=curl_easy_perform(curl);
	  if (res) {
	    printf("Error %d Unable to access the internet\n",res);
	    printf("%s\n",errbuf);
	    return(res);
	  }
	}
	infoptr=strtok_r(info.memory,",",&Exchptr);	//skip symbol
//if (DEBUG) printf("%s\n", infoptr);	
	if (strncmp(infoptr+1+strlen(infoptr), "N/A", 3) == 0) { continue; }  // no name, skip it    
	infoptr=strtok_r(NULL ,"\"",&Exchptr);
//if (DEBUG) printf("%s\n", infoptr);	
	strcpy(name,infoptr);
	infoptr=strtok_r(NULL ,",",&Exchptr);
//if (DEBUG) printf("%s\n", infoptr);	
	if (strncmp(infoptr+1+strlen(infoptr), "N/A",3) == 0) { continue; }  // no exchange, skip it
	strcpy(exchange,infoptr+1);
if (DEBUG) printf("Adding new stock ticker %s\n",Ticker);
	memset(strchr(exchange,'"'),0,1);	// remove the trailing double-quote
	memset(query,0,sizeof(query));
	sprintf(query,"insert into stockinfo (symbol,name,exchange) values(\"%s\",\"%s\",\"%s\")",Ticker,name,exchange);
if (DEBUG) printf("%s\n",query);
	if (!DEBUG) {if (mysql_query(mysql,query)) { printf("%s\n",query); print_error(mysql, "01 Failed to insert new symbol"); } }
    // add pricing history to stockprices
	memset(query,0,sizeof(query));
	sprintf(query, "/Finance/bin/backpop %s %s", Ticker, theYear);
	if (!DEBUG) system(query);
//	mysql_free_result(result);
  }	// end While
  
  return(res);
}	// end GetData

int main(int argc, char * argv[]) {
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  FILE	*sym_list;
  char	*filename="/Finance/gt/Scripts/all-symbols-list.txt";
  char	*perms="w+";
  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NasdaqNM\",\"NGM\", \"NCM\", \"NYSE\", \"NYQ\", \"NSM\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    order by symbol";
/*/    and low_52weeks > "MINPRICE" \
//    and high_52weeks < "MAXPRICE" \
//    and avg_volume > "MINVOLUME" \
//    order by symbol";
*/  
  chunk.memory = calloc(1,1);
  chunk.size = 0;
  info.memory = calloc(1,1);
  info.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) {
    fprintf(stderr,"curl init failed, aborting process\n");
    exit(EXIT_FAILURE);
  }    
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  #include "Includes/beancounter-conn.inc"
  if ((GetData(qAMEX) == 56)) { sleep(2); GetData(qAMEX); }
  if ((GetData(qNASDAQ) == 56)) { sleep(2); GetData(qNASDAQ); }
  if ((GetData(qNYSE) == 56)) { sleep(2); GetData(qNYSE); }
  curl_easy_cleanup(curl);
  free(chunk.memory);
  free(info.memory);
  
  // create the new list
  if (mysql_query(mysql,query_list)) print_error(mysql, "Failed to query database");
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  sym_list=fopen(filename,perms);
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue; }
    fprintf(sym_list,"%s\n",row_list[0]);
  }
  fclose(sym_list);

  // finished with the database
  mysql_free_result(result_list);
  #include "Includes/mysql-disconn.inc"
  
  exit(EXIT_SUCCESS);
}
