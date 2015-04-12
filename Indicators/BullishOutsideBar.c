// BullishOutsideBar.c
/* The OutsidePrevious signal gets triggered if 
 * a security's high is higher than or equal to the previous period's high,
 * the low is lower than or equal to previous period's low,
 * the close is higher than or equal to the previous period's close,
 * and the volume is higher than the previous period's volume.
 * Parms:
 * compile:  gcc -Wall -O2 -ffast-math BullishOutsideBar.c -o BullishOutsideBar `mysql_config --include --libs`
 */

#define		MINVOLUME "80000"
#define		MINPRICE "5"
#define		MAXPRICE "250"

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char	qDate[12];

#include        "../Includes/print_error.inc"
#include	"../Includes/valid_date.inc"

int	main(int argc, char * argv[]) {
// get the list of symbols
  int	num_rows;
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
  float	CurClose,PrevClose,CurLow,PrevLow,CurVolume,PrevVolume,CurHigh,PrevHigh,CurOpen;
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
//  MYSQL_FIELD *field;
  unsigned long	*lengths;
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
  #include "../Includes/beancounter-conn.inc"

  if (mysql_query(mysql,query_list)) print_error(mysql, "Failed to query database");
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 

  //Big Loop through all Symbols
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); break; }
//    valid_date(row_list[0]);
//    sprintf(query,"select day_high,day_low,day_close,volume from stockprices where symbol = \"%s\" and date <= \"%s\" order by date",row_list[0],qDate);
    sprintf(query,"select day_high,day_low,day_close,volume,day_open from stockprices where symbol = \"%s\" and date <= \"%s\" order by date desc limit 2",row_list[0],qDate);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
    num_rows=mysql_num_rows(result);
//    mysql_data_seek(result, num_rows-2);
    
    row=mysql_fetch_row(result);
    // check for nulls
      if(row==NULL) { mysql_free_result(result); continue; }
//      mysql_field_seek(result,0);
//      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { mysql_free_result(result); continue; }
      if (row[0] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[1]) { mysql_free_result(result); continue; }
      if (row[1] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[2]) { mysql_free_result(result); continue; }
      if (row[2] == NULL) { mysql_free_result(result); continue; }
      if (!lengths[3])  { mysql_free_result(result); continue; }
      if (row[3] == NULL) { mysql_free_result(result); continue; }
    
    CurHigh=strtof(row[0],NULL);
    CurLow=strtof(row[1],NULL);
    CurClose=strtof(row[2],NULL);
    CurVolume=strtof(row[3],NULL);
    CurOpen=strtof(row[4],NULL);
    
    row=mysql_fetch_row(result);
    // check for nulls
    if(row==NULL) { mysql_free_result(result); continue; }
//    mysql_field_seek(result,0);
//    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); continue; }
    if (row[0] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[1]) { mysql_free_result(result); continue; }
    if (row[1] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[2]) { mysql_free_result(result); continue; }
    if (row[2] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[3])  { mysql_free_result(result); continue; }
    if (row[3] == NULL) { mysql_free_result(result); continue; }
    if (!lengths[4])  { mysql_free_result(result); continue; }
    if (row[4] == NULL) { mysql_free_result(result); continue; }
    
    PrevHigh=strtof(row[0],NULL);
    PrevLow=strtof(row[1],NULL);
    PrevClose=strtof(row[2],NULL);
    PrevVolume=strtof(row[3],NULL);

// Check on Outside Day
    if ( (CurHigh >= PrevHigh) &&
         (CurClose >= PrevClose) &&
         (CurVolume > PrevVolume) &&
         (CurClose >= CurOpen) &&
         (CurLow <= PrevLow) ) {
	printf("OB\t%s\n",row_list[0]);
    }
    mysql_free_result(result);
  }	// end of Big Loop
  mysql_free_result(result_list);

  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
    

