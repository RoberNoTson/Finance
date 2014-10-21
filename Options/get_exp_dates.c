/* get_current_exp_data.c
 * part of QuoteOptions
 */

#include	"QuoteOptions.h"
#include	"../Includes/ParseData.inc"

int	get_exp_dates(char * Sym) {
  // download the initial data from Yahoo
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

  sprintf(query,"http://finance.yahoo.com/q/op?s=%s",Sym);
  curl_easy_setopt(curl, CURLOPT_URL, query);
  chunk.size = 0;
  res=curl_easy_perform(curl);
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
  // get number of expiration dates we need
  // parse webpage for list of exp dates
  if ((saveptr = strstr(chunk.memory,"View By Expiration:")) == NULL) {
    puts("01 Error parsing data, exiting");
    exit(EXIT_FAILURE);
  }
  if ((saveptr = strstr(saveptr,"<strong>")) == NULL) {
    puts("02 Error parsing current expiration date, exiting");
    exit(EXIT_FAILURE);
  }
  saveptr += strlen("<strong>");	// do we need +1 here?
  if ((endptr = strstr(saveptr,"</strong>")) == NULL) {
    puts("03 Error parsing current expiration date, exiting");
    exit(EXIT_FAILURE);
  }
  memset(cur_exp_date,0,sizeof(cur_exp_date));
  strncpy(cur_exp_date,saveptr,endptr-saveptr);
  if (DEBUG) printf("Current exp date: %s\n",cur_exp_date);
  
  num_exp = 1;
  pExpiration=calloc(1,sizeof(struct EXPIRATION) * num_exp);
  
  // loop to find all "m=yyyy-mm" until we hit "<table" for future dates
  if (current) {
    if ((endptr = strstr(saveptr,"<table")) == NULL) {
      puts("04 Error parsing future expiration dates, exiting");
      exit(EXIT_FAILURE);
    }
    future_exp_dates = calloc(1,1);
    while (saveptr < endptr) {
     if ((saveptr = strstr(saveptr,"m=")) == NULL) break; // no more data
     saveptr += 2;
     num_exp++;
     pExpiration = realloc(pExpiration,sizeof(struct EXPIRATION) * num_exp);
     
     
    }
  }
  
  
  
  return(EXIT_SUCCESS);
}
