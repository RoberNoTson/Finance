// get_bidask.c
// gcc -Wall get_bidask.c -o get_bidask  `curl-config --libs`

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<curl/curl.h>
#include	<ctype.h>
#include	<unistd.h>

struct	MemStruct {
  char *memory;
  size_t size;
};

#include	"Includes/ParseData.inc"

int	main(int argc, char *argv[]) {
  char	*qURL1="http://download.finance.yahoo.com/d/quotes.csv?s=";
  char	*qURL2="&e=.csv&f=b3b2";
  char	qURL[strlen(qURL1)+strlen(qURL2)+(argc>1 ? strlen(argv[1]) : 1)+1];
  char	Bid[10],Ask[10];
  char	Sym[16];
  int	x;
  CURL *curl;
  CURLcode	res;
  struct	MemStruct	chunk;
  
  if (argc != 2) {
    printf("Usage: %s Sym\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  // make symbol uppercase
  memset(Sym,0,sizeof(Sym));
  for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  strcpy(Bid,"N/A");
  strcpy(Ask,"N/A");
  chunk.memory = malloc(1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) exit(EXIT_FAILURE);
  sprintf(qURL,"http://download.finance.yahoo.com/d/quotes.csv?s=%s&e=.csv&f=b3b2",Sym);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_URL, qURL);
  res=curl_easy_perform(curl);
  if (res) {
    sleep(10);
    res=curl_easy_perform(curl);
    if (res) {
      puts("Unable to access the internet");
      exit(EXIT_FAILURE);
    }
  }
  if ((sscanf(chunk.memory," %[0-9.]%*[,]%[0-9.]",Bid,Ask)) == 0) {
    curl_easy_cleanup(curl);
    free(chunk.memory);
    fprintf(stderr,"Error - no data found\n");
    exit(EXIT_FAILURE);
  }
  printf("%s\t%s\n",Bid,Ask);
  curl_easy_cleanup(curl);
  free(chunk.memory);
  exit(EXIT_SUCCESS);
}
