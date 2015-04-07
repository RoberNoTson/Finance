// get_tickers_from_exchange.c
/* list ticker symbols by exchange, returns this formatted data:
 * "Symbol","Name","LastSale","MarketCap","ADR TSO","IPOyear","Sector","industry","Summary Quote",
 * 
 * parms:	nasd | amex | nyse | all
 * compile: gcc -Wall -ffast-math get_tickers_from_exchange.c -o get_tickers_from_exchange `mysql_config --include --libs` `curl-config --libs`
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<curl/curl.h>

struct	MemStruct {
  char *memory;
  size_t size;
};
struct	MemStruct	chunk;

#include	"Includes/ParseData.inc"

char	errbuf[CURL_ERROR_SIZE];
CURL *curl;
CURLcode	res;

void	Usage(char *prog) {
    printf("Usage:  %s {nasd | amex | nyse | all}\n", prog);
    exit(EXIT_FAILURE);
}

int GetData(char *qURL) {
  char 	*saveptr;
  char	Ticker[16];
    curl_easy_setopt(curl, CURLOPT_URL, qURL);
    res=curl_easy_perform(curl);
    if (!res) {
      strtok(chunk.memory ,"\n");	// skip the first line of headers
      while ((saveptr=strtok(NULL,",")) != NULL) {
	strcpy(Ticker,saveptr+1);	// skip the initial double-quote
	memset(strchr(Ticker,'"'),0,1);	// remove the trailing double-quote
	// skip tickers with special characters
	if ((strchr(Ticker,'^')) == NULL && (strchr(Ticker,'/')) == NULL) {
	  printf("%s\n",Ticker);
	}
	strtok(NULL ,"\n");
      }
    } else {
      if (res != 56) {
	printf("Error %d Unable to access the internet\n",res);
	printf("%s\n",errbuf);
      }
    }
    return(res);
}

int main(int argc, char * argv[]) {
  char	*qNASDAQ="http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=nasdaq&render=download";
  char	*qNYSE="http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=nyse&render=download";
  char	*qAMEX="http://www.nasdaq.com/screening/companies-by-name.aspx?letter=0&exchange=amex&render=download";

     // parse cli parms
  if (argc != 2) Usage(argv[0]);
  chunk.memory = calloc(1,1);
  curl=curl_easy_init();
  chunk.size = 0;
  if(!curl) {
    puts("Unable to initialize CURL");
    free(chunk.memory);
    exit(EXIT_FAILURE);
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  if (strcmp(argv[1],"nasd")==0) {
    if ((GetData(qNASDAQ) == 56)) { sleep(2); GetData(qNASDAQ); }
  } else if (strcmp(argv[1],"nyse")==0) {
    if ((GetData(qNYSE) == 56)) { sleep(2); GetData(qNYSE); }
  } else if (strcmp(argv[1],"amex")==0) {
    if ((GetData(qAMEX) == 56)) { sleep(2); GetData(qAMEX); }
  } else if (strcmp(argv[1],"all")==0) {
    if ((GetData(qNASDAQ) == 56)) { sleep(2); GetData(qNASDAQ); }
    if ((GetData(qNYSE) == 56)) { sleep(2); GetData(qNYSE); }
    if ((GetData(qAMEX) == 56)) { sleep(2); GetData(qAMEX); }
  } else {
    curl_easy_cleanup(curl);
    free(chunk.memory);
    Usage(argv[0]);
  }
  curl_easy_cleanup(curl);
  free(chunk.memory);
  exit(EXIT_SUCCESS);
}
