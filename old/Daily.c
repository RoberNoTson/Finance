// Daily.c
/* daily trade evaluator for next day activity, based on closing price trend and ChandelierUp.
 * 
 * Parms: [Date] is optional
 * compile:  gcc -Wall -O2 -ffast-math Daily.c -o Daily `mysql_config --include --libs` `curl-config --libs`
 */

#define		MINVOLUME "80000"
#define		MINPRICE "14"
#define		MAXPRICE "250"


#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<curl/curl.h>

struct	MemStruct {
  char *memory;
  size_t size;
};

MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];

static size_t ParseBidAsk(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemStruct *mem = (struct MemStruct *)userp;
    
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {	// out of memory!
      printf("not enough memory to realloc\n");
      exit(EXIT_FAILURE);
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size = realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

#include        "print_error.inc"
#include	"valid_date.inc"
#include	"holiday_check.inc"

int	main(int argc, char * argv[]) {
#include	"Daily.h"
  
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
  #include "beancounter-conn.c"
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
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseBidAsk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  daily_txt=fopen(filename,perms);
  
  //Big Loop through all Symbols
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); break; }
    valid_date(row_list[0]);
    strcpy(query,"select day_high,day_low,day_open,day_close,previous_close from stockprices where symbol = \"");
    strcat(query,row_list[0]);
    strcat(query,"\" order by date");
    if (mysql_query(mysql,query)) {
      print_error(mysql, "Failed to query database");
    }
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) {
      print_error(mysql, "store_results failed");
    } 
    num_rows=mysql_num_rows(result);
    mysql_data_seek(result, num_rows-3);

    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[3])  { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[3] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[4])  { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[4] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    Prev2High=strtof(row[0],NULL);
    Prev2Low=strtof(row[1],NULL);
    Prev2Open=strtof(row[2],NULL);
    Prev2Close=strtof(row[3],NULL);

    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[3])  { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[3] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    PrevHigh=strtof(row[0],NULL);
    PrevLow=strtof(row[1],NULL);
    PrevOpen=strtof(row[2],NULL);
    PrevClose=strtof(row[3],NULL);

    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"NULL data found, aborting run"); }
    CurHigh=strtof(row[0],NULL);
    CurLow=strtof(row[1],NULL);
    ChanUp=0;
    // check if trending up...
    if ( // New tops each day
	CurHigh >= PrevHigh &&
	PrevHigh > Prev2High &&
	// Each bottom is higher
	CurLow > PrevLow &&
	PrevLow > Prev2Low &&
	// The two first candle are white
	Prev2Close > Prev2Open &&
	PrevClose > PrevOpen ) {	// trending up, check Chandelier
    // get MaxInPeriod of Highs
    StartRow=mysql_num_rows(result)-MinMax_Periods;
    Max=0.0;
    mysql_data_seek(result, StartRow);
    while ((row=mysql_fetch_row(result))) {
      tval = strtof(row[0],NULL);
      if (tval>Max) Max=tval;
    }
    // get MinInPeriod of Lows
    StartRow=mysql_num_rows(result)-MinMax_Periods;
    Min=FLT_MAX;
    mysql_data_seek(result, StartRow);
    while ((row=mysql_fetch_row(result))) {
      tval = strtof(row[1],NULL);
      if (tval<Min) Min=tval;
    }
    // get ATR
    StartRow=mysql_num_rows(result)-ATR_Periods;
    mysql_data_seek(result, StartRow);
    ATR = 0;
    while ((row=mysql_fetch_row(result))) {
      CurHigh=strtof(row[0],NULL);
      CurLow=strtof(row[1],NULL);
      CurClose=strtof(row[3],NULL);
      PrevClose=strtof(row[4],NULL);
      A=CurHigh-CurLow;
      B=fabs(PrevClose - CurHigh);
      C=fabs(PrevClose - CurLow);
      TR=fmax(A,B);
      TR=fmax(TR,C);
      ATR += TR;
    }
    ATR /= ATR_Periods;
    ChanUp = Max-(Coeff*ATR);
    }	// end of ...Check Chandelier
    
    mysql_data_seek(result,num_rows-1);
    row=mysql_fetch_row(result);
    
    if (ChanUp  > strtof(row[3],NULL)) {	// this is a candiate, process the symbol
      ChanDn = Min+(Coeff*ATR);
      // calculate the Pivot Point
      CurHigh = strtof(row[0], NULL);
      CurLow = strtof(row[1], NULL);
      CurClose = strtof(row[3], NULL);
      PP = (CurHigh + CurLow + CurClose) / 3;
      R1 = (PP*2)-CurLow;
      S1 = (PP*2)-CurHigh;
      R2 = (PP-S1)+R1;
      S2 = PP-(R1-S1);
      // get Safe values
      // find last_sticky_high/low, StartRow is (qDate-1)-Safe_Stickyness
      StartRow = num_rows-1-Safe_Stickyness;
      mysql_data_seek(result, StartRow);
      last_sticky_low = last_sticky_high = StartRow;
      for(x=StartRow; x<num_rows-1; x++) {
	row=mysql_fetch_row(result);
	if (row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	// check for nulls
	mysql_field_seek(result,0);
	field = mysql_fetch_field(result);
	lengths=mysql_fetch_lengths(result);
	if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	// find lowest High in range(StartRow::qDate-1)
	if (strtof(row[0],NULL) <= Min_val) {
	  Min_val=strtof(row[0],NULL);
	  last_sticky_low=x;
	}
	// find highest Low in range(StartRow::qDate-1)
	if (strtof(row[1],NULL) >= Max_val) {
	  Max_val=strtof(row[1],NULL);
	  last_sticky_high=x;
	}
      }  // end for
      // scan Period days prior to last_sticky_high for lower lows, save the diffs
      StartRow=last_sticky_high-Safe_Periods;
      mysql_data_seek(result, StartRow);
      row=mysql_fetch_row(result);
      if (row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
      sum_up=0;
      num_high_diff=0;
      for(x=StartRow; x<last_sticky_high; x++) {
	mysql_field_seek(result,1);
	field = mysql_fetch_field(result);
	lengths=mysql_fetch_lengths(result);
	if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	tval=strtof(row[1],NULL);
	row=mysql_fetch_row(result);
	// check for nulls
	if (row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	mysql_field_seek(result,1);
	field = mysql_fetch_field(result);
	lengths=mysql_fetch_lengths(result);
	if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if(strtof(row[1],NULL)<tval) {
	  sum_up += (tval-strtof(row[1],NULL));
	  num_high_diff++;
	} // end if
      } // end for
      // scan Period days prior to last_sticky_low for higher highs, save the diffs
      StartRow=last_sticky_low-Safe_Periods;
      mysql_data_seek(result, StartRow);
      row=mysql_fetch_row(result);
      // check for nulls
      if (row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
      mysql_field_seek(result,0);
      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
      if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
      sum_dn = 0;
      num_low_diff = 0;
      for(x=StartRow; x<last_sticky_low; x++) {
	tval=strtof(row[0],NULL);
	row=mysql_fetch_row(result);
	// check for nulls
	if (row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	mysql_field_seek(result,0);
	field = mysql_fetch_field(result);
	lengths=mysql_fetch_lengths(result);
	if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if(strtof(row[0],NULL)>tval) {
	  sum_dn += (strtof(row[0],NULL)-tval);
	  num_low_diff++;
	}
      } // end for
      // average the diffs, if any
      avg_up = (num_high_diff>1) ? sum_up/(float)num_high_diff : sum_up;
      avg_dn = (num_low_diff>1) ? sum_dn/(float)num_low_diff : sum_dn;
  
      StartRow=num_rows-Safe_Stickyness-1;
      mysql_data_seek(result, StartRow);
      safeup_val=0;
      safedn_val=FLT_MAX;
      for(x=StartRow; x<num_rows-1; x++) {
	row=mysql_fetch_row(result);
	// check for nulls
	if (row==NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	mysql_field_seek(result,0);
	field = mysql_fetch_field(result);
	lengths=mysql_fetch_lengths(result);
	if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
	safeup_val = fmax(safeup_val, strtof(row[1],NULL)-(avg_up*(float)Safe_Coeff));
	safedn_val = fmin(safedn_val, strtof(row[0],NULL)+(avg_dn*(float)Safe_Coeff));
      } // end for
      
      // get Bid/Ask prices
      if(curl) {
	strcpy(qURL,qURL1);
	strcat(qURL, row_list[0]);
	strcat(qURL,qURL2);
	curl_easy_setopt(curl, CURLOPT_URL, qURL);
	chunk.size = 0;
	res=curl_easy_perform(curl);
	if (!res) {
	  strtok(chunk.memory,",");
	  strcpy(Bid,chunk.memory);
	  if ((saveptr=strtok(NULL ,"\015"))==NULL) {
	    printf("strtok error, aborting\n%s\n",chunk.memory);
	    free(chunk.memory);
	    curl_easy_cleanup(curl);
	    mysql_free_result(result_list);
	    #include "mysql-disconn.c"
	    exit(EXIT_FAILURE);
	  }
	  strcpy(Ask,saveptr);
	} else {
	strcpy(Bid,"0.00"); strcpy(Ask,"0.00");
      } }
      fprintf(daily_txt,"Trend\t%s\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%s\t$%.2f\t$%.2f\t$%.2f\t$%s\n",row_list[0],PP,S2,safeup_val,ChanUp,Bid,R2,safedn_val,ChanDn,Ask);
    } // end if candidate

    mysql_free_result(result);
  }	// end of Big Loop

  fclose(daily_txt);
  free(chunk.memory);
  curl_easy_cleanup(curl);
  // finished with the database
  mysql_free_result(result_list);
  #include "mysql-disconn.c"
  exit(EXIT_SUCCESS);
}
