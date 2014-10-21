/* output.c
 * part of my-trades
 */
#include	"./my-trades.h"

int	do_output(char *thisDate) {
  FILE	*file;
  char	*filename="/Finance/Investments/my-trades.list";
  char	*perms="w+";
  char	query[1024];
  int	num_rows,x;
  char	*get_trades="select concat(if(OB=true,'OB',''), \
      case when (KR=true && OB=true) then ',KR' \
	when (KR=true && OB=false) then 'KR' else '' end, \
      case when (TREND=true && (KR=true || OB=true)) then ',Trend' \
	when (TREND=true && KR=false && OB=false) then 'Trend' else '' end, \
      case when (MACD=true && (KR=true || OB=true || TREND=true)) then ',MACD' \
	when (MACD=true && KR=false && OB=false && TREND=false) then 'MACD' else '' end, \
      case when (HV=true && (TREND=true || MACD=true || KR=true || OB=true)) then ',HV' \
	when (HV=true && TREND=false && MACD=false && KR=false && OB=false) then 'HV' else '' end), \
      SYMBOL, PP, S2, SAFEUP, CHANUP, BID, R2, SAFEDN, CHANDN, ASK from TRADES";

// extract and print collected data
  file=fopen(filename,perms);
  if (mysql_query(mysql,get_trades)) { print_error(mysql, "Failed to query TRADES database"); }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed for TRADES"); } 
  num_rows=mysql_num_rows(result);
  while ((row=mysql_fetch_row(result))) {
    fprintf(file,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",row[0],row[1],row[2],row[3],row[4],row[5],row[6],row[7],row[8],row[9],row[10]);
  }
  fclose(file);
  mysql_free_result(result);    
  
// update watchlist with the selected trades
  sprintf(query,"replace into Investments.watchlist \
    (SYMBOL,OB,KR,TREND,MACD,HV,PP,R1,R2,S1,S2,CHANUP,CHANDN,SAFEUP,SAFEDN,BID,ASK,date) \
    select *,\"%s\" from TRADES",thisDate);
  mysql_query(mysql,query);
  mysql_query(mysql,"drop temporary table if exists TRADES");
  #include "../Includes/mysql-disconn.inc"

  // email results
  sprintf(query,"cat %s |mailx -s \"Trade Selections\" mark_roberson@tx.rr.com",filename);
  system(query);
  // output a XLS format file
  sprintf(query,"ssconvert %s ",filename);
  strncat(query,filename,strrchr(filename,'.')-filename);
  strcat(query,".xls");
  if ((x=system(query)) != 0) {
    printf("Spreadsheet conversion failed due to error %d\n",WEXITSTATUS(x));
  }
  return EXIT_SUCCESS;
}
