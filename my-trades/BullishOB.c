/* BullishOB.c
 * part of my-trades
 */
#include	"./my-trades.h"
int	BullishOB(char * Sym) {
  float	CurClose,PrevClose,CurLow,PrevLow,CurVolume,PrevVolume,CurHigh,PrevHigh;
  char	query[1024];
  int	num_rows,x;
  unsigned long	*lengths;

  // process Bullish Outside Bars
    sprintf(query,"select day_high,day_low,day_close,volume from stockprices where symbol = \"%s\" order by date",Sym);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
    num_rows=mysql_num_rows(result);
    if(num_rows<20) { 
      fprintf(stderr,"Too few rows for %s in Bullish Outside Bar\n",Sym); 
      mysql_free_result(result); 
      return(EXIT_FAILURE);
    }
    mysql_data_seek(result, num_rows-2);
    row=mysql_fetch_row(result);
    // check for nulls
    if(row==NULL) { fprintf(stderr,"NULL data found, skipping %s\n",Sym); mysql_free_result(result); return(EXIT_FAILURE); }
    mysql_field_seek(result,0);
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { fprintf(stderr,"NULL data found, skipping %s\n",Sym); mysql_free_result(result); return(EXIT_FAILURE); }
    }
    PrevHigh=strtof(row[0],NULL);
    PrevLow=strtof(row[1],NULL);
    PrevClose=strtof(row[2],NULL);
    PrevVolume=strtof(row[3],NULL);
    row=mysql_fetch_row(result);
    // check for nulls
    if(row==NULL) { fprintf(stderr,"NULL data found, skipping %s\n",Sym); mysql_free_result(result); return(EXIT_FAILURE); }
    mysql_field_seek(result,0);
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { fprintf(stderr,"NULL data found, skipping %s\n",Sym); mysql_free_result(result); return(EXIT_FAILURE); }
    }
    CurHigh=strtof(row[0],NULL);
    CurLow=strtof(row[1],NULL);
    CurClose=strtof(row[2],NULL);
    CurVolume=strtof(row[3],NULL);
    // find Bullish Outside Day
    if ( (CurHigh >= PrevHigh) &&
         (CurClose >= PrevClose) &&
         (CurVolume > PrevVolume) &&
         (CurLow <= PrevLow) ) {
      sprintf(query,"insert into TRADES (SYMBOL) VALUES(\"%s\")",Sym);
      if (mysql_query(mysql,query)) {
	if (mysql_errno(mysql) != 1062) 
	  print_error(mysql, "Failed to insert symbol into temp database");
      }
      sprintf(query,"update TRADES set OB = true where SYMBOL = \"%s\"",Sym);
      if (mysql_query(mysql,query)) print_error(mysql, "Failed to update symbol into temp database");
    } // end if OB candidate
    mysql_free_result(result);
    // end of OB check
    return EXIT_SUCCESS;
}
