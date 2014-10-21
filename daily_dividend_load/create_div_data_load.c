// create_div_data_load.c
/* use http://finance.yahoo.com/q/ks?s="$Symbol"+Key+Statistics" to get Div data
 * Create SQL to load Dividend data for a stock symbol into the Investments.Dividends table
 * Does NOT actually run any updates to the database.
 * Assumes that current data is more accurate than previous version and 
 * will update in place where symbol, exDiv date and DivPay date all match.
 * 
 * compile: gcc -Wall -O2 -ffast-math create_div_data_load.c -o create_div_data_load `mysql_config --include --libs` `curl-config --libs`
 */

#define	DEBUG	0
#define _XOPENSOURCE
#define		MAX_PERIODS	200
#define		MINVOLUME "80000"
#define		MINPRICE "14"
#define		MAXPRICE "250"

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>
#include	<curl/curl.h>

struct	MemStruct {
  char *memory;
  size_t size;
};
MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];
char    errbuf[CURL_ERROR_SIZE];

#include	"/Finance/bin/C/src/Includes/print_error.inc"
#include	"/Finance/bin/C/src/Includes/valid_date.inc"
#include	"/Finance/bin/C/src/Includes/ParseData.inc"
  
int	main(int argc, char * argv[]) {

  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NasdaqNM\",\"NGM\", \"NCM\", \"NYSE\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  char	*qURL1="http://finance.yahoo.com/q/ks?s=\"";
  char	*qURL2="\"+Key+Statistics";
  char	qURL[1024];
  char	*div_query;
  char	*saveptr;
  char	buf[1024];
  char	DivRate[24];
  char	DivYield[24];
  char	ExDivDate[24];
  char	DivPayDate[24];
  CURL *curl;
  CURLcode	res;
  struct	MemStruct	chunk;
  time_t t;
  struct tm *TM;

    // parse cli parms
  if (argc > 2) {
    printf("Usage:  %s [yyyy-mm-dd]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc==2) {
    strcpy(qDate, argv[1]);
  } else {
    t = time(NULL);
    TM = localtime(&t);
    if (TM == NULL) {
      perror("localtime");
      if (mysql != NULL) mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
    if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
      fprintf(stderr, "strftime returned 0");
      if (mysql != NULL) mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
  }

 // connect to the database
  #include "/Finance/bin/C/src/Includes/beancounter-conn.inc"
  if (mysql_query(mysql,query_list)) {
    print_error(mysql, "Failed to query database");
  }
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) {
    print_error(mysql, "store_results failed");
  } 

  div_query = calloc(2,1);
  memset(div_query,0,sizeof(div_query));
  chunk.memory = calloc(1,1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  //Big Loop through all Symbols
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); break; }
    valid_date(row_list[0]);

    memset(div_query,0,sizeof(div_query));
    chunk.size = 0;
    
    if(curl) {
      sprintf(qURL,"%s%s%s",qURL1,row_list[0],qURL2);
      curl_easy_setopt(curl, CURLOPT_URL, qURL);
      res=curl_easy_perform(curl);
    } 
    if (!curl || res) {
	if (DEBUG) printf("Error %d Unable to access the internet\nRetrying %s...\n",res,row_list[0]);
	sleep(5);
	res=curl_easy_perform(curl);
	if (res) {	// error, no data retrieved
	  printf("%s\n",errbuf);
	  puts("curl error, no data retrieved");
	  free(chunk.memory);
	  free(div_query);
	  #include "/Finance/bin/C/src/Includes/mysql-disconn.inc"
	  mysql_free_result(result_list);
	  exit(EXIT_FAILURE);
	}
    }

    memset(DivRate,0,sizeof(DivRate));
    memset(DivYield,0,sizeof(DivYield));
    memset(ExDivDate,0,sizeof(ExDivDate));
    memset(DivPayDate,0,sizeof(DivPayDate));
  
    if ((saveptr=strstr(chunk.memory,"Forward Annual Dividend Rate")) == NULL) {
     continue;
    }
    strncpy(DivRate,saveptr+97,4);
  
    if ((saveptr=strstr(chunk.memory,"Forward Annual Dividend Yield")) == NULL) {
     continue;
    }
    strncpy(DivYield,saveptr+98,4);

    if ((saveptr=strstr(chunk.memory,"Ex-Dividend Date")) == NULL) {
      continue;
    }
    strncpy(ExDivDate,saveptr+92,5);
    if ((saveptr=strchr(ExDivDate,'<')) != NULL) strncpy(saveptr,saveptr+1,1);
    memset(buf,0,sizeof(buf));
    strncpy(buf,strstr(chunk.memory,"Ex-Dividend Date")+85,3);  
    if (!strcasecmp(buf,"JAN")) strcat(ExDivDate,"-01-");
    else if (!strcasecmp(buf,"FEB")) strcat(ExDivDate,"-02-");
    else if (!strcasecmp(buf,"MAR")) strcat(ExDivDate,"-03-");
    else if (!strcasecmp(buf,"APR")) strcat(ExDivDate,"-04-");
    else if (!strcasecmp(buf,"MAY")) strcat(ExDivDate,"-05-");
    else if (!strcasecmp(buf,"JUN")) strcat(ExDivDate,"-06-");
    else if (!strcasecmp(buf,"JUL")) strcat(ExDivDate,"-07-");
    else if (!strcasecmp(buf,"AUG")) strcat(ExDivDate,"-08-");
    else if (!strcasecmp(buf,"SEP")) strcat(ExDivDate,"-09-");
    else if (!strcasecmp(buf,"OCT")) strcat(ExDivDate,"-10-");
    else if (!strcasecmp(buf,"NOV")) strcat(ExDivDate,"-11-");
    else if (!strcasecmp(buf,"DEC")) strcat(ExDivDate,"-12-");
    else strcat(ExDivDate,"-00-");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%02d",atoi(strstr(chunk.memory,"Ex-Dividend Date")+89));
    strcat(ExDivDate,buf);
    if ((saveptr=strstr(chunk.memory,"Dividend Date")) == NULL) {
      continue;
    }
    strncpy(DivPayDate,saveptr+89,5);
    if ((saveptr=strchr(DivPayDate,'<')) != NULL) strncpy(saveptr,saveptr+1,1);
    memset(buf,0,sizeof(buf));
    strncpy(buf,strstr(chunk.memory,"Dividend Date")+82,3);
    if (!strcasecmp(buf,"JAN")) strcat(DivPayDate,"-01-");
    else if (!strcasecmp(buf,"FEB")) strcat(DivPayDate,"-02-");
    else if (!strcasecmp(buf,"MAR")) strcat(DivPayDate,"-03-");
    else if (!strcasecmp(buf,"APR")) strcat(DivPayDate,"-04-");
    else if (!strcasecmp(buf,"MAY")) strcat(DivPayDate,"-05-");
    else if (!strcasecmp(buf,"JUN")) strcat(DivPayDate,"-06-");
    else if (!strcasecmp(buf,"JUL")) strcat(DivPayDate,"-07-");
    else if (!strcasecmp(buf,"AUG")) strcat(DivPayDate,"-08-");
    else if (!strcasecmp(buf,"SEP")) strcat(DivPayDate,"-09-");
    else if (!strcasecmp(buf,"OCT")) strcat(DivPayDate,"-10-");
    else if (!strcasecmp(buf,"NOV")) strcat(DivPayDate,"-11-");
    else if (!strcasecmp(buf,"DEC")) strcat(DivPayDate,"-12-");
    else strcat(DivPayDate,"-00-");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%02d",atoi(strstr(chunk.memory,"Dividend Date")+86));
    strcat(DivPayDate,buf);
    // create Div data load SQL
    sprintf (buf,"REPLACE INTO Investments.Dividends () VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",DEFAULT,DEFAULT);\n",
    row_list[0],DivRate,DivYield,ExDivDate,DivPayDate);
    // skip "N/A" lines
    if (strstr(buf,"N/A")) {
      continue;
    }
    div_query = realloc(div_query, strlen((char *)div_query)+strlen(buf)+1);
    if (div_query == NULL) {
      puts("not enough memory to realloc div_query");
      exit(EXIT_FAILURE);
    }
    strcat(div_query,buf);
  
    printf("%s",div_query);
  }	// end of Big Loop
  mysql_free_result(result_list);

  curl_easy_cleanup(curl);
  free(chunk.memory);
  free(div_query);
// finished with the database
  #include "/Finance/bin/C/src/Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
