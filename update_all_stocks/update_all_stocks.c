// update_all_stocks.c
/* Update of today's (or most recent available) data to stockinfo, stockprices for all active stock symbols
 * Sometimes Yahoo provides bad dates in their data; you can force a correction to the most recent business day 
 * by passing the [force] parm. 
 * This program should normally be run on the same business day, after market close.
 * NOTE: be cautious with "force" as data will be overwritten with no validity checks
 * 
 * Parms: [force]
 * compile: gcc -Wall -O2 -ffast-math -o update_all_stocks update_all_stocks.c `mysql_config --include --libs` `curl-config --libs`
 */

#include	"update_all_stocks.h"

#include        "../Includes/print_error.inc"
#include        "../Includes/ParseData.inc"

int	debug=DEBUG;
int force=0;
struct        MemStruct chunk;

int main(int argc, char * argv[]) {
  char	query[1024];
  char	Sym[16];
  MYSQL	*mysql;
  MYSQL_ROW	row;
  MYSQL_RES *result;
  int x,y,rem,num_rows=0;
  pthread_t     t_parse_update[MAXPARA];

  // parse cli parms
  if (argc >2) Usage(argv[0]);
  // is it a valid parm?
  if (argc == 2) {
    if (strcasecmp(argv[1],"force")) Usage(argv[0]);
    // otherwise turn on the "force" flag to coerce invalid dates to this one
    force++;
  }

  if (time_check() == EXIT_FAILURE) exit(EXIT_FAILURE);
  
  // allocate memory and initialize things
  if ((chunk.memory = calloc(1,1)) == NULL) {
    puts("chunk calloc failed in scan_db, aborting process");
    return(EXIT_FAILURE);
  }    
  // get list of active stocks to update
  #include      "/Finance/bin/C/src/Includes/beancounter-conn.inc"
  sprintf(query,"select distinct SYMBOL from beancounter.stockinfo where active");
  if (mysql_query(mysql,query)) { puts("Failed to query stockinfo database in update_all_stocks"); return(EXIT_FAILURE); }
  if ((result=mysql_store_result(mysql)) == NULL) { puts("Failed to store results for stockinfo database in update_all_stocks"); return(EXIT_FAILURE); }
  if ((num_rows=mysql_num_rows(result)) == 0) { puts("No rows found in update_all_stocks"); return(EXIT_FAILURE); }
  rem = num_rows % MAXPARA;

  // multi-thread version
  for (x=0;x<(num_rows-rem);x+=MAXPARA) {
    for (y=0;y<MAXPARA;y++) {
      if ((row=mysql_fetch_row(result)) == NULL) break;
      strcpy(Sym,row[0]);
      t_parse_update[y] = fork();
      if (t_parse_update[y] == -1) {
	perror("fork attempt failed...");
	exit(EXIT_FAILURE);
      }
      if (t_parse_update[y] == 0) {
	parse_update(Sym);
	exit(EXIT_SUCCESS);
      }
    }	// end FOR(Y...
    for (y=0;y<MAXPARA;y++) {
      waitpid(t_parse_update[y],NULL,0);
    }
  }	// end FOR(X...
  if (debug) puts("Processing the rem stuff");
  for (x=(num_rows-rem);x<num_rows;x+=MAXPARA) {
    for (y=0;y<rem;y++) {
      if ((row=mysql_fetch_row(result)) == NULL) break;
      strcpy(Sym,row[0]);
      t_parse_update[y] = fork();
      if (t_parse_update[y] == -1) {
	perror("fork attempt failed...");
	exit(EXIT_FAILURE);
      }
      if (t_parse_update[y] == 0) {
	parse_update(Sym);
	exit(EXIT_SUCCESS);
      }
    }	// end FOR(Y...
    for (y=0;y<rem;y++) {
      waitpid(t_parse_update[y],NULL,0);
    }
  }	// end FOR(X...

  // finished with the database
  mysql_free_result(result);
  #include "../Includes/mysql-disconn.inc"
  free(chunk.memory);
  exit(EXIT_SUCCESS);
}
