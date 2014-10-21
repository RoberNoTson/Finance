// sym_change.c
/* update database when ticker symbols change
 * Parms: none
 * compile:  gcc -Wall -O2 -ffast-math sym_change.c -o sym_change `mysql_config --include --libs` `curl-config --libs`
 * 
 * source "http://www.nasdaq.com/markets/stocks/symbol-change-history.aspx?sortby=EFFECTIVE&descending=Y"
 * extract the "genTable" rows by grepping for "<td class=\"body2\""
 * 	- 3 lines for each change: old_symbol new_symbol eff_date
 * 
 * <td class="body2">ABD.WI        </td>
 * <td class="body2"><a id="main_content_rptSymbols_newsymbol_0" href="http://www.nasdaq.com/symbol/acco.wi">ACCO.WI</a></td>
 * <td class="body2">04/25/2012</td>
 * 
 * <td class="body2">BILB          </td>
 * <td class="body2"><a id="main_content_rptSymbols_newsymbol_1" href="http://www.nasdaq.com/symbol/bilbe">BILBE</a></td>
 * <td class="body2">04/25/2012</td>
 * 
 * <td class="body2">BMODE         </td>
 * <td class="body2"><a id="main_content_rptSymbols_newsymbol_2" href="http://www.nasdaq.com/symbol/bmod">BMOD</a></td>
 * <td class="body2">04/25/2012</td>
 * 
 * remove all "<td class="body2">", then all "</td>", then all </a>
 * search backwards from the end to the last ">", whatever is found is the new_symbol
 * trim white space
 * 
 */

#define		DEBUG	0
#define		_XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"

struct	MemStruct {
  char *memory;
  size_t size;
};
#include        "Includes/ParseData.inc"

int main(int argc, char * argv[]) {
  MYSQL_RES *result;
//  MYSQL_ROW row;
  char	errbuf[CURL_ERROR_SIZE];
  char	query[1024];
  char	old_symbol[64];
  char	new_symbol[64];
  char	eff_date[64];
  char	*saveptr;
  char	*endptr;
  time_t	t;
  struct tm	*TM;
  CURL *curl;
  CURLcode	res;
  struct	MemStruct chunk;
  
  t = time(NULL);
  TM = localtime(&t);
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
  curl_easy_setopt(curl, CURLOPT_URL,"http://www.nasdaq.com/markets/stocks/symbol-change-history.aspx?sortby=EFFECTIVE&descending=Y");
  res=curl_easy_perform(curl);
  if (res) {	// 0 means no errors
      if (res == 56 || res == 28) {	// retry
	sleep(5);
	res=curl_easy_perform(curl);
	if (res) {
	  printf("Error %d Unable to access the internet\n",res);
	  printf("%s\n",errbuf);
	  exit(EXIT_FAILURE);
	}
      } else {	// retry
	printf("Error %d Unable to access the internet\n",res);
	printf("%s\n",errbuf);
	exit(EXIT_FAILURE);
      }
  }

  if ((saveptr = strstr(chunk.memory,"genTable")) == NULL) {
    puts("Error parsing data, exiting");
    exit(EXIT_FAILURE);
  }
  if ((endptr = strstr(chunk.memory,"END genTable")) == NULL) {
    puts("Error parsing data, exiting");
    exit(EXIT_FAILURE);
  }
 
  // Big Loop through table
  while (saveptr < endptr) {
    // old symbol
    if ((saveptr = strstr(saveptr,"<td class=\"body2\">")) == NULL) break;	// no more data
    sscanf(saveptr,"<td class=\"body2\">%s",old_symbol);
    saveptr += strlen("<td class=\"body2\">");
    // new symbol
    if ((saveptr = strstr(saveptr,"http://www.nasdaq.com/symbol/")) == NULL) break;	// no more data
    saveptr += strlen("http://www.nasdaq.com/symbol/");
    saveptr = strchr(saveptr,'>')+1;
    memset(strchr(saveptr,'<'),0,1);
    strcpy(new_symbol,saveptr);
    saveptr += strlen(new_symbol)+1;
    // effective date
    if ((saveptr = strstr(saveptr,"<td class=\"body2\">")) == NULL) break;	// no more data
    sscanf(saveptr,"<td class=\"body2\">%s<",eff_date);
    saveptr += strlen("<td class=\"body2\">");
    // fix date format
    strptime(saveptr,"%m/%d/%Y",TM);
    strftime(eff_date,sizeof(eff_date),"%F",TM);
    saveptr += strlen(eff_date)+1;
    // see if update is needed
    #include "Includes/beancounter-conn.inc"
    sprintf(query,"select symbol from stockinfo where symbol = \"%s\"",old_symbol);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed"); 
    if (mysql_num_rows(result) == 0) {	// ticker not present, no action needed
      if (DEBUG) printf("%s not found in stockinfo\n",old_symbol);
      mysql_free_result(result);
      continue;
    }
    // see if the new symbol is already present 
    sprintf(query,"select symbol from stockinfo where symbol = \"%s\"",new_symbol);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql, "store_results failed"); 
    if (mysql_num_rows(result)) {	// ticker exists, can't change the old one
      if (DEBUG) printf("New symbol %s already exists in stockinfo, can't update %s. Please correct manually.\n",new_symbol,old_symbol);
      mysql_free_result(result);
      continue;
    }
    
    // do the updates
    sprintf(query,"update stockinfo set symbol = \"%s\" where symbol = \"%s\"",new_symbol,old_symbol);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if (mysql_query(mysql,query)) {
      printf("Failed to update stockinfo, no change from %s to %s\n",old_symbol,new_symbol);
      if (mysql != NULL && mysql_errno(mysql)) fprintf (stderr, "Error %u: %s\n", mysql_errno(mysql), mysql_error(mysql));
      continue;
    }
    sprintf(query,"update stockinfo set active = 1 where symbol = \"%s\"",new_symbol);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if (mysql_query(mysql,query)) {
      printf("Failed to update stockinfo, no change to active status for %s\n",new_symbol);
      if (mysql != NULL && mysql_errno(mysql)) fprintf (stderr, "Error %u: %s\n", mysql_errno(mysql), mysql_error(mysql));
      continue;
    }
    sprintf(query,"update stockprices set symbol = \"%s\" where symbol = \"%s\"",new_symbol,old_symbol);
    if (DEBUG) printf("%s\n",query);
    if (!DEBUG) if (mysql_query(mysql,query)) {
      printf("Failed to update stockprices, no change from %s to %s\n",old_symbol,new_symbol);
      if (mysql != NULL && mysql_errno(mysql)) fprintf (stderr, "Error %u: %s\n", mysql_errno(mysql), mysql_error(mysql));
      continue;
    }
    printf("Symbol changed from %s to %s\n",old_symbol,new_symbol);
  }	// end Whie (Big Loop)

  curl_easy_cleanup(curl);
  free(chunk.memory);
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  
  exit(EXIT_SUCCESS);
}
