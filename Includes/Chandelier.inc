// Chandelier.inc
/* 
 * The Chandelier Exit provides Stops for closing Long or Short positions
 */
#ifndef	_CHANDELIER_INC
#define	_CHANDELIER_INC 1

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>

int Chandelier(char *Sym,char *qDate) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  char query[1024];
  float Max,Min,trow;
  float	ATR=0;
  float PrevHigh,PrevLow,PrevClose,PrevPrevClose,A,B,C,TR;
  int	Coeff=3;
  int	ATR_Periods=22;
  int	MinMax_Periods=22;
  int	StartRow;

  // get MaxInPeriod of PrevHighs
  sprintf(query,"select day_high from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) { puts("Failed to query database in Chandelier"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("store_results failed in Chandelier"); return(EXIT_FAILURE); }
  StartRow=mysql_num_rows(result)-MinMax_Periods;
  Max=0.0;
  mysql_data_seek(result, StartRow);
  while ((row=mysql_fetch_row(result))) {
      trow = strtof(row[0],NULL);
      if (trow>Max) Max=trow;
  }
  mysql_free_result(result);
  // get MinInPeriod of PrevLows
  sprintf(query,"select day_low from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) {puts("Failed to query database in Chandelier"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("store_results failed in Chandelier"); return(EXIT_FAILURE); }
  StartRow=mysql_num_rows(result)-MinMax_Periods;
  Min=FLT_MAX;
  mysql_data_seek(result, StartRow);
  while ((row=mysql_fetch_row(result))) {
      trow = strtof(row[0],NULL);
      if (trow<Min) Min=trow;
  }
  mysql_free_result(result);
  // get ATR
  sprintf(query,"select day_high,day_low,day_close,previous_close from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) {puts("Failed to query database in Chandelier"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("store_results failed in Chandelier"); return(EXIT_FAILURE); }
  StartRow=mysql_num_rows(result)-ATR_Periods;
  mysql_data_seek(result, StartRow);
  while ((row=mysql_fetch_row(result))) {
      PrevHigh=strtof(row[0],NULL);
      PrevLow=strtof(row[1],NULL);
      PrevClose=strtof(row[2],NULL);
      PrevPrevClose=strtof(row[3],NULL);
      A=PrevHigh-PrevLow;
      B=fabs(PrevPrevClose - PrevHigh);
      C=fabs(PrevPrevClose - PrevLow);
      TR=fmax(A,B);
      TR=fmax(TR,C);
      ATR += TR;
  }
  mysql_free_result(result);
  ATR /= ATR_Periods;
  // calculate and print ChadUp/Down 
  ChanUp=Max-(Coeff*ATR);
  ChanDn=Min+(Coeff*ATR);
  return(EXIT_SUCCESS);
}
#endif
