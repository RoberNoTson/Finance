/* BullishKR.c
 * part of my-trades
 */
#include	"./my-trades.h"
int	BullishKR(char * Sym) {
  int num_rows,x,StartRow,y;
  int	bad_row=0;
  float	Prev2Close,Prev3Close,sum;
  float	CurClose,PrevClose,CurLow,PrevLow,CurVolume,PrevVolume,CurHigh,PrevHigh;
  char	query[1024];
  unsigned long	*lengths;

  sprintf(query,"select day_close,previous_close,day_low,volume from stockprices where symbol = \"%s\" order by date",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
  if(result==NULL) {
      fprintf(stderr,"03 Skipping NULL data for %s\n",Sym); 
      delete_bad(Sym);
      mysql_free_result(result); 
      return(EXIT_FAILURE); 
  }
  num_rows=mysql_num_rows(result);
  if(num_rows<14) { 
      fprintf(stderr,"Too few rows for %s in Bullish Key Reversal\n",Sym); 
      delete_bad(Sym);
      mysql_free_result(result); 
      return(EXIT_FAILURE); 
  }
    // calculate 14 day average volume prior to target date
    StartRow = num_rows-14;
    sum = 0;
    bad_row=0;
    mysql_data_seek(result, StartRow);
    for (x=StartRow; x<num_rows; x++) {
      row=mysql_fetch_row(result);
      if (row==NULL) { bad_row++; break; }
      mysql_field_seek(result,0);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[3]) { bad_row++; break; }
      sum += strtof(row[3],NULL);
    }
    if (bad_row>0) { 
      fprintf(stderr,"04 NULL volume found at row %d, skipping %s\n",x,Sym); 
      delete_bad(Sym);
      mysql_free_result(result); 
      return(EXIT_FAILURE); 
    }
    sum /= 14;
    mysql_data_seek(result, num_rows-3);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"05 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    mysql_field_seek(result,0);
    lengths=mysql_fetch_lengths(result);
    for (y=0;y<mysql_num_fields(result);y++) {
      if (!lengths[y]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"05 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE); }
    }    
    Prev3Close=strtof(row[1],NULL);
    Prev2Close=strtof(row[0],NULL);
    row=mysql_fetch_row(result);
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"05 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    // error check for nulls
    mysql_field_seek(result,0);
    lengths=mysql_fetch_lengths(result);
    for (y=0;y<mysql_num_fields(result);y++) {
      if (!lengths[y]) { fprintf(stderr,"05 Skipping NULL data for %s\n",Sym); mysql_free_result(result); return(EXIT_FAILURE); }
    }
    PrevLow=strtof(row[2],NULL);
    row=mysql_fetch_row(result);
    if(row==NULL) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"05 Skipping NULL data for %s\n",Sym); return(EXIT_FAILURE);}
    // error check for nulls
    mysql_field_seek(result,0);
    lengths=mysql_fetch_lengths(result);
    for (y=0;y<mysql_num_fields(result);y++) {
      if (!lengths[y]) { fprintf(stderr,"05 Skipping NULL data for %s\n",Sym); mysql_free_result(result); return(EXIT_FAILURE); }
    }
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
      sprintf(query,"insert into TRADES (SYMBOL) VALUES(\"%s\")",Sym);
      if (mysql_query(mysql,query)) {
	if (mysql_errno(mysql) != 1062) 
	  print_error(mysql, "Failed to insert symbol into temp database");
      }
      sprintf(query,"update TRADES set KR = true where SYMBOL = \"%s\"",Sym);
      if (mysql_query(mysql,query)) { print_error(mysql, "Failed to update symbol into temp database"); }
    } // end if KR candidate
  mysql_free_result(result);
  // end of KR check
  return EXIT_SUCCESS;
}
