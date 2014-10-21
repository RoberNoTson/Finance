// BOL-test.c -- Bollinger band calc test
/* 
 * Parms: (Sym, Periods, coeff, Type, date)
 * extern	float bolsup, bolinf
 */
#ifndef	_BOL_INC
#define	_BOL_INC 1

#define		MAX_PERIODS	200
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<errno.h>


int	BOL(char *Sym, int Periods, float coeff, char * Type, char * qDate) {
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  unsigned long	*lengths;
  char	query[1024];
  int	StartRow,num_rows,x,y;
  int	qType=3;
  float	sum,sd,sd_sum=0,sma=0;

 // connect to the database
  #include "./beancounter-conn.inc"
  if (strcasecmp(Type, "open") == 0) qType=0;
  if (strcasecmp(Type, "low") == 0) qType=1;
  if (strcasecmp(Type, "high") == 0) qType=2;
  if (strcasecmp(Type, "close") == 0) qType=3;

  // query database for required values
  sprintf(query,"select day_open,day_low,day_high,day_close,date from stockprices where symbol = \"%s\" \
  and date <= %s order by date asc",Sym,strcasecmp(qDate,"null") ? "\"qDate\"" : "NOW()");
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol %s\n",Sym);
    mysql_close(mysql);
    return(EXIT_FAILURE);
  }
  if (num_rows < (Periods+1)) {
    printf("Not enough rows found to process Standard Deviation for %d periods\n",Periods);
    mysql_close(mysql);
    return(EXIT_FAILURE);
  }

  // good data retrieved, begin processing
  // Get Periods SMA value
  StartRow=num_rows-Periods;
  mysql_data_seek(result, StartRow);
  sum=0.0;
  for (x=0;x<Periods;x++) {
      row=mysql_fetch_row(result);
      // error check for nulls
      if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
      mysql_field_seek(result,qType);
      field = mysql_fetch_field(result);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[qType]) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
      if (row[qType] == NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
      sum += strtof(row[qType], NULL);
  }
  sma = sum/Periods;

  mysql_data_seek(result, StartRow);
  sd_sum=0.0;
  for (x=0;x<Periods;x++) {
    row=mysql_fetch_row(result);
    if(row==NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[qType]) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
    if (row[qType] == NULL) { mysql_free_result(result); fprintf(stderr,"Oops!  null data found and skipped\n"); continue; }
    sd_sum += pow((strtof(row[qType],NULL)-sma,2);
  }
  sd = sqrtf(sd_sum/Periods);
  bolsup = sma + (coeff * sd);
  bolinf = sma - (coeff * sd);

  // finished with the database
  mysql_free_result(result);
  #include "mysql-disconn.inc"
  return(EXIT_SUCCESS);
}
#endif
