/* TrendUp.c
 * part of my-trades
 */
#include	"./my-trades.h"
int	TrendUp(char * Sym) {
  int num_rows,x,StartRow;
  int	bad_row=0;
  float	Prev2Close,Prev3Close,sum;
  float	CurClose,PrevClose,CurLow,PrevLow,CurVolume,PrevVolume,CurHigh,PrevHigh;
  char	query[1024];
  unsigned long	*lengths;
  int	ATR_Periods=22;
  float	Prev2High,Prev2Low,PrevOpen,Prev2Open;
  
    // find TrendUp candidates
    sprintf(query,"select day_high,day_low,day_open,day_close,previous_close from stockprices where symbol = \"%s\" order by date",Sym);
    if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
    result=mysql_store_result(mysql);
    if ((result==NULL) && (mysql_errno(mysql))) {
      print_error(mysql, "store_results failed");
    } 
    num_rows=mysql_num_rows(result);
    if (num_rows<ATR_Periods+1) {
      fprintf(stderr,"Too few rows for %s in TrendUp check\n",Sym); 
      delete_bad(Sym);
      mysql_free_result(result); 
      return(EXIT_FAILURE);
    }
    mysql_data_seek(result, num_rows-3);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"07 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"07 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    }
    Prev2High=strtof(row[0],NULL);
    Prev2Low=strtof(row[1],NULL);
    Prev2Open=strtof(row[2],NULL);
    Prev2Close=strtof(row[3],NULL);

    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"07 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"07 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    }
    PrevHigh=strtof(row[0],NULL);
    PrevLow=strtof(row[1],NULL);
    PrevOpen=strtof(row[2],NULL);
    PrevClose=strtof(row[3],NULL);

    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"07 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    lengths=mysql_fetch_lengths(result);
    for (x=0;x<mysql_num_fields(result);x++) {
      if (!lengths[x]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"07 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    }
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
      Chandelier(Sym,qDate);
    }	// end of ...Check Chandelier
    
    mysql_data_seek(result,num_rows-1);
    row=mysql_fetch_row(result);
    
    if (ChanUp  > strtof(row[3],NULL)) {	// this is a candidate, process the symbol
      sprintf(query,"insert into TRADES (SYMBOL) VALUES(\"%s\")",Sym);
      if (mysql_query(mysql,query)) {
	if (mysql_errno(mysql) != 1062) 
	  print_error(mysql, "Failed to insert symbol into temp database");
	}
      sprintf(query,"update TRADES set Trend = true where SYMBOL = \"%s\"",Sym);
      if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update symbol into temp database"); }
    } // end if candidate
    mysql_free_result(result);    
    // end of TrendUp check
  return EXIT_SUCCESS;
}
