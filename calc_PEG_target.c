// calc_PEG_target.c
/* calculate a target price based on GARP/PEG values
 * formula:  PE*EPS and (PE*EPS)-Last
 * Parms: Sym
 * output:  Sym Target Change
 * 
 * gcc -Wall -O2 calc_PEG_target.c -o calc_PEG_target `curl-config --libs`
 */

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <curl/curl.h>
#include        <ctype.h>
#include        <unistd.h>

struct  MemStruct {
  char *memory;
  size_t size;
};

#include        "Includes/ParseData.inc"

int     main(int argc, char *argv[]) {
  char  *qURL1="http://download.finance.yahoo.com/d/quotes.csv?s=";
  char  *qURL2="&e=.csv&f=l1re";
  char  qURL[strlen(qURL1)+strlen(qURL2)+(argc>1 ? strlen(argv[1]) : 1)+1];
  char  Sym[16];
  float  Last=0,PE=0,EPS=0;
  int   x;
  CURL *curl;
  CURLcode      res;
  struct        MemStruct       chunk;
  
  if (argc != 2) {
    printf("Usage: %s Sym\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  // make symbol uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  chunk.memory = malloc(1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) exit(EXIT_FAILURE);
  sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=l1re",Sym);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
  res=curl_easy_perform(curl);
  if (res) {
    sleep(5);
    res=curl_easy_perform(curl);
    if (res) {
      puts("Unable to access the internet");
      exit(EXIT_FAILURE);
    }
  }
  // parse the data
  if ((sscanf(chunk.memory," %f%*[,]%f%*[,]%f",&Last,&PE,&EPS)) == 0) {
    curl_easy_cleanup(curl);
    free(chunk.memory);
    fprintf(stderr,"Error - no data found\n");
    exit(EXIT_FAILURE);
  }

  // calculate and print output here
  printf("%s\t%.2f\t%.2f\n",Sym,PE*EPS,(PE*EPS)-Last);
  curl_easy_cleanup(curl);
  free(chunk.memory);
  exit(EXIT_SUCCESS);
}

