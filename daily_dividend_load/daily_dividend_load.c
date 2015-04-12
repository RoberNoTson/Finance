/* daily_div_load.c
 * 
 * Use http://finance.yahoo.com/q/ks?s="$Symbol"+Key+Statistics" to get Div data
 * Create SQL to load Dividend data for a stock symbol into the Investments.Dividends table
 * Assumes that current data is more accurate than previous version and 
 * will update in place where symbol, exDiv date and DivPay date all match.
 * 
 * NOTE: http://download.finance.yahoo.com/d/quotes.csvr?e=.csv&f=dyqr1&s= gives BAD DATA
 * NOTE: error 1146 is returned when the ExDiv date is later than the PayDate
 * 
 * compile: gcc -Wall -O2 -ffast-math daily_dividend_load.c -o daily_dividend_load `mysql_config --include --libs` `curl-config --libs`
 */

#include	"./daily_dividend_load.h"
#include	"../Includes/print_error.inc"
//#include	"../Includes/valid_date.inc"
#include	"../Includes/holiday_check.inc"
#include	"../Includes/ParseData.inc"

int	main(int argc, char * argv[]) {

  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NYSE\",\"NMS\",\"PCX\",\"NYQ\",\"NCM\",\"ASE\",\"NasdaqNM\",\"WCB\",\"PNK\",\"NIM\",\"NGM\") \
    and active = true \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  char	qURL[1024];
  char	buf[1024];
  char	Sym[12];
  char	DivRate[24];
  char	DivYield[24];
  char	ExDivDate[24];
  char	DivPayDate[24];
  char	*saveptr;
  int	x;
  CURL *curl;
  CURLcode	res;
  time_t t,t2;
  struct tm *TM, *TM2;

  if (argc > 2) {
    printf("Usage: %s [yyyy-mm-dd]\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  // initialize time structures
  t = time(NULL);
  TM = localtime(&t);
  TM2 = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    if (mysql != NULL) mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (TM2 == NULL) {
    perror("localtime");
    if (mysql != NULL) mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (argc==2) {
    strcpy(qDate,argv[1]);
  } else {
    if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
      fprintf(stderr, "strftime returned 0");
      if (mysql != NULL) mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
  }
  strptime(qDate,"%F",TM);

 // connect to the database
  #include "../Includes/beancounter-conn.inc"
  holiday_check(qDate);
  if (mysql_query(mysql,query_list)) {
    print_error(mysql, "Failed to query database");
  }
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) {
    print_error(mysql, "store_results failed");
  } 

  chunk.memory = calloc(1,1);
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  //Big Loop through all Symbols
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad or missing data in daily_dividend_load\n"); continue; }
    memset(Sym,0,sizeof(Sym));
    for (x=0;x<strlen(row_list[0]);x++) Sym[x]=toupper(row_list[0][x]);
//    valid_date(Sym);
    chunk.size = 0;
    // download data for the symbol
    if(curl) {
      sprintf(qURL,"http://finance.yahoo.com/q/ks?s=%s+Key+Statistics",Sym);
      curl_easy_setopt(curl, CURLOPT_URL, qURL);
      res=curl_easy_perform(curl);
    } 
    if (!curl || res) {
	printf("Error %d Unable to access the internet\nRetrying %s...\n",res,Sym);
	sleep(15);
	res=curl_easy_perform(curl);
	if (res) {	// error, no data retrieved
	  print_error(mysql,"error, no data retrieved");
	  printf("for %s\n",Sym);
      curl_easy_cleanup(curl);
      free(chunk.memory);
      mysql_free_result(result_list);
      #include "../Includes/mysql-disconn.inc"
      exit(EXIT_FAILURE);
	}
    }
    
    memset(DivRate,0,sizeof(DivRate));
    memset(DivYield,0,sizeof(DivYield));
    memset(ExDivDate,0,sizeof(ExDivDate));
    memset(DivPayDate,0,sizeof(DivPayDate));
    memset(TM,0,sizeof(TM));
    memset(TM2,0,sizeof(TM));
    
    if ((saveptr=strstr(chunk.memory,"Forward Annual Dividend Rate")) == NULL) continue;
    saveptr = strstr(saveptr,"yfnc_tabledata1\">");
    saveptr += strlen("yfnc_tabledata1\">");
    strncpy(DivRate,saveptr,(strchr(saveptr,'<'))-saveptr);
    if (strstr(DivRate,"N/A")) continue;
    
    if ((saveptr=strstr(chunk.memory,"Forward Annual Dividend Yield")) == NULL) continue;
    saveptr = strstr(saveptr,"yfnc_tabledata1\">");
    saveptr += strlen("yfnc_tabledata1\">");
    strncpy(DivYield,saveptr,(strchr(saveptr,'<'))-saveptr);
    if (strstr(DivYield,"N/A")) continue;
    if ((saveptr=strchr(DivYield,'%'))) memset(saveptr,0,1);

    if ((saveptr=strstr(chunk.memory,"Dividend Date")) == NULL) continue;
    saveptr = strstr(saveptr,"yfnc_tabledata1\">");
    saveptr += strlen("yfnc_tabledata1\">");
    strncpy(DivPayDate,saveptr,(strchr(saveptr,'<'))-saveptr);
    if (DEBUG) printf("DivPayDate: %s\n",DivPayDate);
    if (strstr(DivPayDate,"N/A")) continue;
    xlate_date(DivPayDate);

    if ((saveptr=strstr(chunk.memory,"Ex-Dividend Date")) == NULL) continue;
    saveptr = strstr(saveptr,"yfnc_tabledata1\">");
    saveptr += strlen("yfnc_tabledata1\">");
    strncpy(ExDivDate,saveptr,(strchr(saveptr,'<'))-saveptr);
    if (DEBUG) printf("ExDivDate: %s\n",ExDivDate);
    if (strstr(ExDivDate,"N/A")) continue;
    xlate_date(ExDivDate);
    // compare dates for correct sequence, ExDiv < DivPay
    strptime(ExDivDate,"%F",TM);
    strptime(DivPayDate,"%F",TM2);
    if ((t = mktime(TM)) == -1) { printf("Error parsing TM for %s\n",Sym); continue; }
    if ((t2 = mktime(TM2)) == -1) { printf("Error parsing TM for %s\n",Sym); continue; }
    if (t > t2) {
      printf("Skipping invalid date values for %s:\tExDate %s\tPayDate %s\n",Sym,ExDivDate,DivPayDate);
      continue;
    }
    
    // create Div data load SQL
    sprintf (buf,"REPLACE INTO Investments.Dividends () VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",DEFAULT,DEFAULT);\n",
      Sym,DivRate,DivYield,ExDivDate,DivPayDate);
    if (DEBUG) printf("%s\n",buf);
    if (!DEBUG) { if (mysql_query(mysql,buf) && (mysql_errno(mysql) != 1146))  printf("Failed to update dividends for %s, error %d\n%s\n",row_list[0],mysql_errno(mysql),buf); }
  }	// end of Big Loop
  mysql_free_result(result_list);
  
  printf("Investments.Dividends update completed\n");
  
  // finished with the database, free memory
  curl_easy_cleanup(curl);
  free(chunk.memory);
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
