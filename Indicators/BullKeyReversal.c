// BullKeyReversal.c
// Scan a list of symbols, print out the ones that match the criteria.
/* The BullKeyReversal signal gets triggered if:
 * a security's close is higher than the previous period's close, 
 * the previous close is lower than the 2 previous days' close (downtrend), 
 * the low is less than the previous period's low,
 * the volume is higher than the 14 day average volume,
 * Parms:
 * compile: gcc -Wall -O2 -ffast-math BullKeyReversal.c -o BullKeyReversal `mysql_config --include --libs`
 */

#define		MINVOLUME "80000"
#define		MINPRICE "14"
#define		MAXPRICE "250"

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<errno.h>
#include	<ctype.h>

  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char	qDate[12];

#include        "../Includes/print_error.inc"
#include	"../Includes/valid_date.inc"


int	main(int argc, char * argv[]) {
// get the list of symbols
  int	num_rows,StartRow,x;
  int	bad_row=0;
  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NYSE\",\"NMS\",\"PCX\",\"NYQ\",\"NCM\",\"ASE\",\"NasdaqNM\",\"WCB\",\"PNK\",\"NIM\",\"NGM\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  char	query[200];
  float	CurClose,PrevClose,Prev2Close,Prev3Close,CurLow,PrevLow,CurVolume,sum;
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  MYSQL_FIELD *field;
  time_t t;
  struct tm *TM;
  unsigned long	*lengths;
  
 // connect to the database
  #include "../Includes/beancounter-conn.inc"
  
  if (argc==2) {
    strcpy(qDate, argv[1]);
  } else {
    t = time(NULL);
    TM = localtime(&t);
    if (TM == NULL) {
      perror("localtime");
      exit(EXIT_FAILURE);
    }
    if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
      fprintf(stderr, "strftime returned 0");
      exit(EXIT_FAILURE);
    }
  }
  if (mysql_query(mysql,query_list)) print_error(mysql, "Failed to query database for symbol list");
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL)) print_error(mysql, "store_result_list failed");

  //Big Loop through all Symbols
  while ((row_list=mysql_fetch_row(result_list)) != NULL) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); break; }
//    valid_date(row_list[0]);
    strcpy(query,"select day_close,previous_close,day_low,volume from stockprices where symbol = \"");
    strcat(query,row_list[0]);
    strcat(query,"\" and date <= \"");
    strcat(query,qDate);
    strcat(query,"\" order by date");
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    result=mysql_store_result(mysql);
    if ((mysql_errno(mysql))) print_error(mysql, "store_results failed");
    if(result==NULL) { mysql_free_result(result); fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue; }
    num_rows=mysql_num_rows(result);
    if(num_rows<14) { mysql_free_result(result); fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue; }
    // calculate 14 day average volume prior to target date
    StartRow = num_rows-14;
    sum = 0;
    mysql_data_seek(result, StartRow);
    
    bad_row=0;
    for (x=StartRow; x<num_rows; x++) {
      row=mysql_fetch_row(result);
      if(row==NULL) break;
      mysql_field_seek(result,3);
      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[3]) { bad_row++; fprintf(stderr,"null volume found, skipping %s\n",row_list[0]); break; }
      sum += strtof(row[3],NULL);
    }
    
    if (bad_row) { mysql_free_result(result); continue; }
    sum /= 14;
    
    mysql_data_seek(result, num_rows-3);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); continue; }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); continue; }
    if (row[0] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[1]) { mysql_free_result(result); continue; }
    if (row[1] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[2]) { mysql_free_result(result); continue; }
    if (row[2] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[3])  { mysql_free_result(result); continue; }
    if (row[3] == NULL) { mysql_free_result(result); continue; }
    
    Prev3Close=strtof(row[1],NULL);
    Prev2Close=strtof(row[0],NULL);
    row=mysql_fetch_row(result);
    if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue;}
    // error check for nulls
      mysql_field_seek(result,0);
      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { mysql_free_result(result); continue; }
      if (row[0] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[1]) { mysql_free_result(result); continue; }
      if (row[1] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[2]) { mysql_free_result(result); continue; }
      if (row[2] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[3])  { mysql_free_result(result); continue; }
      if (row[3] == NULL) { mysql_free_result(result); continue; }

    PrevLow=strtof(row[2],NULL);
    row=mysql_fetch_row(result);
    if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue;}
    // error check for nulls
      mysql_field_seek(result,0);
      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { mysql_free_result(result); continue; }
      if (row[0] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[1]) { mysql_free_result(result); continue; }
      if (row[1] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[2]) { mysql_free_result(result); continue; }
      if (row[2] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[3])  { mysql_free_result(result); continue; }
      if (row[3] == NULL) { mysql_free_result(result); continue; }

    CurClose=strtof(row[0],NULL);
    PrevClose=strtof(row[1],NULL);
    CurLow=strtof(row[2],NULL);
    CurVolume=strtof(row[3],NULL);

    // Get Bullish Key Reversal
    if ( (CurClose > PrevClose) &&
         (PrevClose < Prev2Close) &&
         (Prev2Close <= Prev3Close) &&
         (CurLow <= PrevLow) &&
         (CurVolume > sum) ) {
      printf("KR\t%s\n",row_list[0]);
    }
    mysql_free_result(result);
  }	// end of Big Loop
  mysql_free_result(result_list);

  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
