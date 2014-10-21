/* get_RTquote.c
 * part of watchlist_monitor
 * 
 */
#include "watchlist_monitor.h"

float	get_RTquote(char * sym) {
  char	query[1024];
  char	Price[16];
  char	errbuf[CURL_ERROR_SIZE];
  char 	*saveptr;
  char	*qURL="http://www.google.com/finance?q=";	// concat exchange:Sym like "NYSE:KO"
  int	x,loop;
  CURL *curl;
  CURLcode	res=0;
  float cur_price=0.0;
  
  // get the online data and return/display it
  if (debug) printf("Getting RT quote for %s\n",sym);
  sprintf(query,"%s%s",qURL,sym);
  curl=curl_easy_init();
  if(!curl) {
    puts("curl init failed, aborting process");
    return(EXIT_FAILURE);
  }    
  chunk.size = 0;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);	// set 1 for multithread, 0 for single thread
//  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);	// > CURLOPT_CONNECTTIMEOUT, total time to wait before quit
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_URL, query);
  res=curl_easy_perform(curl);
  if (res) {	// 0 means no errors
    if (res != 56) {
      if (debug) printf("01 Error %d Unable to access the internet\n",res);
      if (debug) printf("%s\n",errbuf);
      curl_easy_cleanup(curl);
      return(0);
    } else {	// retry
	res=curl_easy_perform(curl);
	if (res) {
	  if (debug) printf("02 Error %d Unable to access the internet after retry\n",res);
	  if (debug) printf("%s\n",errbuf);
	  curl_easy_cleanup(curl);
	  return(0);
	}
    }
  }
    // was any data returned?
  if (strstr(chunk.memory,"404 Not Found")) {
    if (debug) printf("%s not found in get_RTquote\n",sym);
    curl_easy_cleanup(curl);
    return 0.0;
  }
  // do the token parsing here
  if ((saveptr = strstr(chunk.memory,"<span class=\"pr\">")) == NULL) { 
    if (debug) printf("Bad HTML, no token found in %d bytes for %s\n",chunk.size,sym);
    curl_easy_cleanup(curl);
    return 0.0;
  }
  saveptr += strlen("<span class=\"pr\">")+1;
  saveptr = strchr(saveptr,'>')+1;	// should now be pointing at the current price
  memset(Price,0,sizeof(Price));
  strncpy(Price,saveptr,(strchr(saveptr,'<'))-saveptr);
  cur_price = strtof(Price,NULL);
  if (debug) printf("%.2f\n",cur_price);
  curl_easy_cleanup(curl);
  return cur_price; 
}
