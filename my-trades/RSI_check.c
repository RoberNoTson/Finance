/* RSI-check.c
 * part of my-trades
 */
#include	"./my-trades.h"

float	RSI_check(char *Sym) {
  float	diff,rsi,rs=0.0;
  float	CurClose,PrevClose,CurLow,CurHigh;
  int	RSI_Periods=14;
  float	won_points=0.0,lost_points=0.0;
  int	num_rows,x;
  unsigned long	*lengths;
  int	bad_row=0;
  char	query[1024];
  
  sprintf(query,"select day_close, previous_close from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query database"); }
    result=mysql_store_result(mysql);       // save the query results
    if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
    num_rows=mysql_num_rows(result);
    if (num_rows < (RSI_Periods+1)) {
      fprintf(stderr,"Too few rows found to process RSI for %d periods for %s\n",RSI_Periods,Sym);
      delete_bad(Sym);
      mysql_free_result(result);
      return 100;
    }
    mysql_data_seek(result, num_rows-RSI_Periods);
    row=mysql_fetch_row(result);
    // check for nulls
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",Sym); return 100; }
    mysql_field_seek(result,0);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",Sym); return 100; }
    if (!lengths[1]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",Sym); return 100; }
    CurClose=strtof(row[0],NULL);
    PrevClose=strtof(row[1],NULL);
    diff = CurClose - PrevClose;
    // Add the won points
    if (diff > 0) {
      won_points += diff;
    }
    // Add the lost points
    if (diff < 0) {
      lost_points -= diff;
    }
    PrevClose=CurClose;
    bad_row=0;
    for(x = num_rows-RSI_Periods+1; x < num_rows; x++) {
      row=mysql_fetch_row(result);
      // check for nulls
      if(row==NULL) { bad_row++; break; }
      mysql_field_seek(result,0);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[0]) { bad_row++; break; }
      CurClose=strtof(row[0],NULL);
      diff = CurClose - PrevClose;
      // Add the won points
      if (diff > 0) {
	won_points += diff;
      }
      // Add the lost points
      if (diff < 0) {
	lost_points -= diff;
      }
      PrevClose=CurClose;
    }	// end For
    mysql_free_result(result);
    if (bad_row) { delete_bad(Sym); fprintf(stderr,"Null data found, skipping %s\n",Sym); return 100; }
    if (lost_points != 0) {
      rs = won_points / lost_points;
    }
    rsi = 100 - (100 / (1 + rs));
  return rsi;
}
