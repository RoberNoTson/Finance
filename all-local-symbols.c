// all-local-symbols.c
/* create a list of all active symbols in the local MySQL database
 * save output to /Finance/gt/Scripts/all-symbols-list.txt for use by other programs/scripts
 * 
 * Parms:  none
 * compile: gcc -Wall -O2 -ffast-math all-local-symbols.c -o all-local-symbols `mysql_config --include --libs`
 */

#define		MINVOLUME "100000"
#define		MINPRICE "14"
#define		MAXPRICE "250"

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"

int	main(int argc, char * argv[]) {
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  FILE	*sym_list;
  char	*filename="/Finance/gt/Scripts/all-symbols-list.txt";
  char	*perms="w+";
  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NYSE\",\"NMS\",\"PCX\",\"NYQ\",\"NCM\",\"ASE\",\"NasdaqNM\",\"WCB\",\"PNK\",\"NIM\",\"NGM\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  
  #include "Includes/beancounter-conn.inc"
  if (mysql_query(mysql,query_list)) print_error(mysql, "Failed to query database");
  result_list=mysql_store_result(mysql);
  if ((result_list==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed");
  sym_list=fopen(filename,perms);
  while ((row_list=mysql_fetch_row(result_list))) {
    if(row_list == NULL) { fprintf(stderr,"Skipping bad data for %s\n",row_list[0]); continue; }
    fprintf(sym_list,"%s\n",row_list[0]);
  }
  fclose(sym_list);
  // finished with the database
  mysql_free_result(result_list);
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}  // end of program
