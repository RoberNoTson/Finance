// PP.inc - calculate Pivot values
// Usage:  PP(Symbol,dateStr,&pp,&r1,&r2,&s1,&s2)
/* use CURRENT_DATE unless a 'yyyy-mm-dd' passed as cli parm
 * print only the most current data unless a number is passed as cli parm
 * The calculation for the new day are calculated from the High (H), low (L) and close (C) of the previous day.
 * Pivot point = P = (H + L + C)/3
 * First area of resistance = R1 = 2P - L
 * First area of support = S1 = 2P - H
 * Second area of resistance = R2 = (P -S1) + R1
 * Second area of support = S2 = P - (R1 - S1)
 * 
 * compile: gcc -Wall -O2 -ffast-math -o Pivot-predict Pivot-predict.c `mysql_config --include --libs`
 */
#ifndef _PP_INC
#define	_PP_INC 1

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>

int Pivot(char *Sym,char *qDate) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  double	CurHigh, CurLow, CurClose;
  char query[1024];
//  MYSQL_FIELD *field;
  unsigned long	*lengths;
  
  // get the prices
  sprintf(query,"select day_high, day_low, day_close, date from stockprices where symbol = \"%s\" and date <= \"%s\" order by date",Sym,qDate);
  if (mysql_query(mysql,query)) { puts("Failed to query database in PP"); return(EXIT_FAILURE); }
  // save the query results
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) { puts("store_results failed in PP"); return(EXIT_FAILURE);  } 
  // fetch the last row
  mysql_data_seek(result,mysql_num_rows(result)-1);
  row=mysql_fetch_row(result);
  if (row==NULL) { puts("fetch_row failed in PP"); return(EXIT_FAILURE);  }
  // check for nulls
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0]) { mysql_free_result(result); puts("Null data found, abandoning process PP"); return(EXIT_FAILURE); }
  if (row[0] == NULL) { mysql_free_result(result); puts("Null data found, abandoning process PP"); return(EXIT_FAILURE); }
  if (!lengths[1]) { mysql_free_result(result); puts("Null data found, abandoning process PP"); return(EXIT_FAILURE); }
  if (row[1] == NULL) { mysql_free_result(result); puts("Null data found, abandoning process PP"); return(EXIT_FAILURE); }
  if (!lengths[2]) { mysql_free_result(result); puts("Null data found, abandoning process PP"); return(EXIT_FAILURE); }
  if (row[2] == NULL) { mysql_free_result(result); puts("Null data found, abandoning process PP"); return(EXIT_FAILURE); }
  // extract data
  CurHigh = strtof(row[0], NULL);
  CurLow = strtof(row[1], NULL);
  CurClose = strtof(row[2], NULL);
  mysql_free_result(result);
  PP = (CurHigh + CurLow + CurClose) / 3.0;
  R1 = (PP*2)-CurLow;
  S1 = (PP*2)-CurHigh;
  R2 = (PP-S1)+R1;
  S2 = PP-(R1-S1);
  return(EXIT_SUCCESS);
}
#endif	/* !_PP_INC */
