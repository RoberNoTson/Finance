/* MACD_check.c
 * part of my-trades
 */
#include	"./my-trades.h"

int	MACD_check(char * Sym) {
  float	first_ema=0,second_ema=0,third_ema=0,macd=0,alpha1,alpha2,alpha3,tval;
  float	*third_ema_list=NULL;
  int	MACD_Periods=14;
  char	query[1024];
  int	StartRow,num_rows,y,x,bad_row;
  float	Period1=12,Period2=26,Period3=9;
  int	Max_Periods=MAX_PERIODS;
  float	oldema1=0.0,oldema2=0.0,oldema3=0.0,Macd1=0,Macd2=0;
  int	qType=3;
  unsigned long	*lengths;

    sprintf(query,"select day_open,day_low,day_high,day_close,day_change,volume,date from stockprices \
      where symbol = \"%s\" order by date",Sym);
    if (mysql_query(mysql,query)) {
      print_error(mysql, "Failed to query database");
    }
    result=mysql_store_result(mysql);	  // save the query results
    if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed"); } 
    num_rows=mysql_num_rows(result);
    if (num_rows < 20)  {
      fprintf(stderr,"Too few rows found to process MACD for %d periods for %s\n",MACD_Periods,Sym);
      delete_bad(Sym);
      mysql_free_result(result);
      return EXIT_FAILURE;
    }
    // Get the EMA values and calculate and stores the MACD values
    alpha1 = 2 / (float)(Period1 + 1);
    alpha2 = 2 / (float)(Period2 + 1);
    alpha3 = 2 / (float)(Period3 + 1);
    Max_Periods = num_rows > Max_Periods ? Max_Periods : num_rows;
    StartRow = num_rows-Max_Periods;
    if ((third_ema_list=calloc(Max_Periods, sizeof(float)))==NULL) { mysql_free_result(result); print_error(mysql,"calloc failed, aborting!"); }
    //  first_ema 
    mysql_data_seek(result,StartRow);
    row=mysql_fetch_row(result);
    // error check for nulls
    if(row==NULL) {  fprintf(stderr,"NULL data found, skipping %s\n",Sym); mysql_free_result(result); return EXIT_FAILURE; }
    mysql_field_seek(result,qType);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[qType]) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",Sym); return EXIT_FAILURE; }
    // get the starting oldema value  
    oldema1 = oldema2 = oldema3 = strtof(row[qType],NULL);
    // return EXIT_FAILURE on with next row after getting initial oldema
    y=1;
    bad_row=0;
    for(x=StartRow+1; x<num_rows; x++) {
      row=mysql_fetch_row(result);
      // error check for nulls
      if(row==NULL) { bad_row++; break; }
      mysql_field_seek(result,qType);
      lengths=mysql_fetch_lengths(result);
      if (!lengths[qType]) { bad_row++; break; }
      if (!lengths[6]) { bad_row++; break; }
      tval = strtof(row[qType],NULL);
      first_ema = (tval - oldema1) * alpha1 + oldema1;
      second_ema = (tval - oldema2) * alpha2 + oldema2;
      oldema1=first_ema;
      oldema2=second_ema;
      macd = first_ema - second_ema;
      oldema3 = oldema1 - oldema2;
      third_ema = (oldema3 - third_ema) * alpha3 + third_ema;
      oldema3 = third_ema;
      if (x == num_rows-2) Macd2 = macd-third_ema;
      y++;
    }	// end of for loop
    if (bad_row) { delete_bad(Sym); mysql_free_result(result); fprintf(stderr,"Null data found, skipping %s\n",Sym); return EXIT_FAILURE; }
    Macd1 = macd-third_ema;
    if (Macd1 > 0 && Macd2 > 0 && Macd1 > Macd2) {
      sprintf(query,"insert into TRADES (SYMBOL) VALUES(\"%s\")",Sym);
      if (mysql_query(mysql,query)) {
	if (mysql_errno(mysql) != 1062) print_error(mysql, "Failed to insert symbol into temp database");
      }
      sprintf(query,"update TRADES set MACD = true where SYMBOL = \"%s\"",Sym);
      if (mysql_query(mysql,query)) print_error(mysql, "Failed to update symbol into temp database");
    } // end if
    mysql_free_result(result);    

    return EXIT_SUCCESS;
}
