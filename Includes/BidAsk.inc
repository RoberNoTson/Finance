// BidAsk.inc
#ifndef	_Bid_Ask
#define	_Bid_Ask 1
#include	<curl/curl.h>
#include	<ctype.h>

struct	BidAskMemStruct {
  char *memory;
  size_t size;
};

static size_t ParseBidAsk(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct BidAskMemStruct *mem = (struct BidAskMemStruct *)userp;
    
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {	// out of memory!
      puts("not enough memory to realloc");
      exit(EXIT_FAILURE);
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

int	BidAsk(char *Sym) {
  char	qURL[1024];
  char	bid[10],ask[10];
  CURL *curl;
  CURLcode	res;
  struct	BidAskMemStruct	BidAskchunk;
  
  strcpy(bid,"N/A");
  strcpy(ask,"N/A");
  Bid = Ask = 0.0;
  if ((BidAskchunk.memory = malloc(1)) == NULL) return(EXIT_FAILURE);
  BidAskchunk.size = 0;
//  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) {
      free(BidAskchunk.memory);
      return(EXIT_FAILURE);
  }

//  sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=b3b2",Sym);
  sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=ba",Sym);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseBidAsk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&BidAskchunk);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);	// >CURLOPT_CONNECTTIMEOUT, total time to wait
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);	// 1=multithread, 0=single
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
  res=curl_easy_perform(curl);
  if (res) {
    sleep(5);
    res=curl_easy_perform(curl);
    if (res) {
      puts("Unable to access the internet in BidAsk.inc");
      free(BidAskchunk.memory);
      return(EXIT_FAILURE);
    }
  }
  if ((sscanf(BidAskchunk.memory," %[0-9.]%*[,]%[0-9.]",bid,ask)) == EOF) {
    curl_easy_cleanup(curl);
    free(BidAskchunk.memory);
    printf("No bid/ask data found for %s\n",Sym);
    return(EXIT_FAILURE);
  }
  curl_easy_cleanup(curl);
  free(BidAskchunk.memory);
  Bid=strtod(bid,NULL);
  Ask=strtod(ask,NULL);
  return(EXIT_SUCCESS);
}
#endif
