/* do_output.c
 * part of mt2
 */

#include	"./mt2.h"

int	do_output(void) {
  int	x,num_rows;
  FILE	*out_file;
  char	*out_filename="/Finance/Investments/watchlist_summary.txt";
  FILE	*file;
  char	*filename="/Finance/Investments/my-sales.list";
  char	*perms="w+";
  char	query[1024];
  
    // print results, if any
  sprintf(query,"select concat(if(OB=true,'OB',''), \
      case when (KR=true && OB=true) then ',KR' \
	when (KR=true && OB=false) then 'KR' else '' end, \
      case when (TREND=true && (KR=true || OB=true)) then ',Trend' \
	when (TREND=true && KR=false && OB=false) then 'Trend' else '' end, \
      case when (MACD=true && (KR=true || OB=true || TREND=true)) then ',MACD' \
	when (MACD=true && KR=false && OB=false && TREND=false) then 'MACD' else '' end, \
      case when (HV=true && (TREND=true || MACD=true || KR=true || OB=true)) then ',HV' \
	when (HV=true && TREND=false && MACD=false && KR=false && OB=false) then 'HV' else '' end), \
      SYMBOL, R1_NEXT, SAFEDN_NEXT, CHANDN_NEXT, ASK_NEXT \
      from %s.watchlist where PAPER_BUY_PRICE > 0 and date = \"%s\"",DB_INVESTMENTS,priorDate);
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query watchlist database"); }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) { print_error(mysql, "store_results failed for watchlist"); } 
  num_rows=mysql_num_rows(result);
  if (num_rows) {
    file=fopen(filename,perms);
    while ((row=mysql_fetch_row(result))) {
      fprintf(file,"%s\t%s\t%s\t%s\t%s\t%s\n",row[0],row[1],row[2],row[3],row[4],row[5]);
    }
    fclose(file);
    mysql_free_result(result);
    // email results
    sprintf(query,"cat %s |mailx -s \"Trade Summary\" mark_roberson@tx.rr.com",filename);
    if (!DEBUG) system(query);
    // output a XLS format file
    sprintf(query,"ssconvert %s ",filename);
    strncat(query,filename,strrchr(filename,'.')-filename);
    strcat(query,".xls");
    if (!DEBUG) if ((x=system(query)) != 0) {
      printf("Spreadsheet conversion failed due to error %d\n",WEXITSTATUS(x));
    }
  } else printf("No trade data found today\n");
  
  sprintf(query,"select concat(if(OB=true,'OB',''), \
      case when (KR=true && OB=true) then ',KR' \
	when (KR=true && OB=false) then 'KR' else '' end, \
      case when (TREND=true && (KR=true || OB=true)) then ',Trend' \
	when (TREND=true && KR=false && OB=false) then 'Trend' else '' end, \
      case when (MACD=true && (KR=true || OB=true || TREND=true)) then ',MACD' \
	when (MACD=true && KR=false && OB=false && TREND=false) then 'MACD' else '' end, \
      case when (HV=true && (TREND=true || MACD=true || KR=true || OB=true)) then ',HV' \
	when (HV=true && TREND=false && MACD=false && KR=false && OB=false) then 'HV' else '' end), \
      symbol, PAPER_BUY_PRICE, PAPER_SELL_PRICE, PAPER_PL from %s.watchlist \
      where date = \"%s\" and PAPER_PL is not null order by PAPER_PL",DB_INVESTMENTS,prevpriorDate);
  if (mysql_query(mysql,query)) { print_error(mysql, "Failed to query watchlist"); }
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql,"No paper P/L data found");
  
  out_file=fopen(out_filename,perms);
  printf("Paper Trade Results for stocks bought %s, sold today\n",priorDate);
  fprintf(out_file,"Paper Trade Results for stocks bought %s, sold today\n",priorDate);
  printf("%20s\tSym\tCost\tSell\tP/L\n"," ");
  fprintf(out_file,"%20s\tSym\tCost\tSell\tP/L\n"," ");
  while ((row=mysql_fetch_row(result)) != NULL) {
    printf("%20s\t%s\t%s\t%s\t%s\n",row[0],row[1],row[2],row[3],row[4]);
    fprintf(out_file,"%20s\t%s\t%s\t%s\t%s\n",row[0],row[1],row[2],row[3],row[4]);
  }
  sprintf(query,"select sum(PAPER_PL), cast(avg(PAPER_PL) as DECIMAL(10,4)) from %s.watchlist \
    where PAPER_PL is not null and date = \"%s\"",DB_INVESTMENTS,prevpriorDate);
  mysql_query(mysql,query);
  if ((result=mysql_store_result(mysql)) == NULL) print_error(mysql,"no PAPER_PL found");
  row=mysql_fetch_row(result);
  printf("Total P/L today: $%s\tAverage P/L per stock: $%s\n",row[0],row[1]);
  fprintf(out_file,"Total P/L today: $%s\tAverage P/L per stock: $%s\n",row[0],row[1]);
  mysql_free_result(result);
  fclose(out_file);
  // email results
  sprintf(query,"cat %s |mailx -s \"Watchlist Summary\" mark_roberson@tx.rr.com",out_filename);
  if (!DEBUG) system(query);

  return EXIT_SUCCESS;
}
