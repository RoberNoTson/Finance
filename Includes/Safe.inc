// Safe.c
/* the SafeZone stop provides stops for closing long or short positions.
 * It accepts the number of bars to use for the calculation and a coefficient as parameters,
 * with 20 and 2 being the defaults.
 * The last parameter is the number of days a "plateau" is maintained 
 * regardless of prices moving against the trade. 
 * This is to take into account the fact that stops may only be extended in the direction of the trade.
 * After prices have been moving against the trade for the number of bars that is specified by the third parameter
 * it is assumed that the stop was triggered and normal calculation of new stops is resumed.
 */
#ifndef	_SAFE_INC
#define	_SAFE_INC 1

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>


int Safe(char *Sym, char *qDate) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  char	query[200];
  int	Safe_Periods=20;
  int	Safe_Coeff=2;
  int	Safe_Stickyness=6;
  int	num_low_diff=0;
  int	num_high_diff=0;
  int	num_rows,StartRow,x,last_sticky_high,last_sticky_low;
  float	Min_val=FLT_MAX;
  float	Max_val=0;
  float	sum_up=0;
  float	sum_dn=0;
  float	avg_up,avg_dn,tval;
  MYSQL_FIELD *field;
  unsigned long	*lengths;

  SafeUp=0;
  SafeDn=FLT_MAX;
  sprintf(query,"select day_high,day_low from stockprices where symbol = \"%s\" and date <= \"%s\" order by date",Sym,qDate);
  if (mysql_query(mysql,query)) { puts("Failed to query database in SafeZone"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("store_results failed in SafeZone"); return(EXIT_FAILURE); }
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\" in Safe\n",Sym);
    return(EXIT_FAILURE);
  }
  
  if (num_rows < (Safe_Periods+Safe_Stickyness+Safe_Coeff)) {
    puts("Not enough rows found to process SafeZone");
    return(EXIT_FAILURE);
  }
  // find last_sticky_high/low, StartRow is (qDate-1)-Safe_Stickyness
  StartRow = num_rows-1-Safe_Stickyness;
  mysql_data_seek(result, StartRow);
  last_sticky_low = last_sticky_high = StartRow;
  for(x=StartRow; x<num_rows-1; x++) {
    if ((row=mysql_fetch_row(result)) == NULL) {
      printf("0 rows found for symbol \"%s\" in Safe\n", Sym);
      return(EXIT_FAILURE);
    }
    // check for nulls
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { puts("Null data found, abandoning process SafeZone"); return(EXIT_FAILURE); }
    if (row[0] == NULL) { puts("Null data found, abandoning process SafeZone"); return(EXIT_FAILURE); }
    if (!lengths[1]) { puts("Null data found, abandoning process SafeZone"); return(EXIT_FAILURE); }
    if (row[1] == NULL) { puts("Null data found, abandoning process SafeZone"); return(EXIT_FAILURE); }
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
  }
  
// scan Period days prior to last_sticky_high for lower lows, save the diffs
  StartRow=last_sticky_high-Safe_Periods;
  mysql_data_seek(result, StartRow);
  row=mysql_fetch_row(result);
  for(x=StartRow; x<last_sticky_high; x++) {
    if (row==NULL) { puts("fetch_row failed in SafeZone");  return(EXIT_FAILURE); }
    // check for nulls
    mysql_field_seek(result,1);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[1]) { puts("Null data found in Safe, abandoning process"); return(EXIT_FAILURE); }
    if (row[1] == NULL) { puts("Null data found in Safe, abandoning process"); return(EXIT_FAILURE); }
    tval=strtof(row[1],NULL);
    row=mysql_fetch_row(result);
    if (row==NULL) { puts("fetch_row failed in Safe processing"); return(EXIT_FAILURE); }
    // check for nulls
    mysql_field_seek(result,1);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[1]) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if (row[1] == NULL) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if(strtof(row[1],NULL)<tval) {
      sum_up += (tval-strtof(row[1],NULL));
      num_high_diff++;
    }
  }
  // scan Period days prior to last_sticky_low for higher highs, save the diffs
  StartRow=last_sticky_low-Safe_Periods;
  mysql_data_seek(result, StartRow);
  row=mysql_fetch_row(result);
  // check for nulls
  if (row==NULL) { puts("fetch_row failed in Safe processing");  return(EXIT_FAILURE); }
  mysql_field_seek(result,0);
  field = mysql_fetch_field(result);
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0]) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
  if (row[0] == NULL) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
  for(x=StartRow; x<last_sticky_low; x++) {
    tval=strtof(row[0],NULL);
    row=mysql_fetch_row(result);
    // check for nulls
    if (row==NULL) { puts("fetch_row failed in Safe processing");  return(EXIT_FAILURE); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if (row[0] == NULL) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if(strtof(row[0],NULL)>tval) {
      sum_dn += (strtof(row[0],NULL)-tval);
      num_low_diff++;
    }
  }
  // average the diffs, if any
  avg_up = (num_high_diff>1) ? sum_up/(float)num_high_diff : sum_up;
  avg_dn = (num_low_diff>1) ? sum_dn/(float)num_low_diff : sum_dn;
  StartRow=num_rows-Safe_Stickyness-1;
  mysql_data_seek(result, StartRow);
  for(x=StartRow; x<num_rows-1; x++) {
    row=mysql_fetch_row(result);
    // check for nulls
    if (row==NULL) { puts("fetch_row failed in Safe processing");  return(EXIT_FAILURE); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if (row[0] == NULL) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if (!lengths[1]) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    if (row[1] == NULL) { puts("Null data found, abandoning Safe process"); return(EXIT_FAILURE); }
    SafeUp = fmax(SafeUp, strtof(row[1],NULL)-(avg_up*(float)Safe_Coeff));
    SafeDn = fmin(SafeDn, strtof(row[0],NULL)+(avg_dn*(float)Safe_Coeff));
  }
  return(EXIT_SUCCESS);
}
#endif
