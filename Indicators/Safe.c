// Safe.c
/* the SafeZone stop provides stops for closing long or short positions.
 * It accepts the number of bars to use for the calculation and a coefficient as parameters,
 * with 20 and 2 being the defaults.
 * The last parameter is the number of days a "plateau" is maintained 
 * regardless of prices moving against the trade. 
 * This is to take into account the fact that stops may only be extended in the direction of the trade.
 * After prices have been moving against the trade for the number of bars that is specified by the third parameter
 * it is assumed that the stop was triggered and normal calculation of new stops is resumed.
 * 
 * Parms:  Symbol [Safe_Periods Safe_Coefficient Safe_Stickyness]
 * compile:  gcc -Wall -O2 -ffast-math -o Safe Safe.c `mysql_config --include --libs`
 */

#define _XOPENSOURCE
#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

  static MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char qDate[12];

#include	"../Includes/print_error.inc"
#include	"../Includes/valid_sym.inc"
#include	"../Includes/valid_date.inc"

int main(int argc, char *argv[]) {
  char	query[200];
  char	Sym[16];
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
  float	safeup_val=0;
  float	safedn_val=FLT_MAX;
  float	avg_up,avg_dn,tval;
  time_t t;
  struct tm *TM;
  unsigned long	*lengths;

  // connect to the database
  #include "../Includes/beancounter-conn.inc"

  // parse cli parms
  if (argc == 1) {
    printf("Usage:  %s Sym [Safe_Periods(20) Safe_Coeff(2) Safe_Stickyness(6)]\n \
    The SafeZone stop provides stops for closing long or short positions.\n \
    It accepts the number of bars to use for the calculation and a coefficient as parameters,\n \
    with 20 and 2 being the defaults.\n \
    The last parameter is the number of days a \"plateau\" is maintained\n \
    regardless of prices moving against the trade.\n",
    argv[0]);
    exit(EXIT_FAILURE);
  }
  if (argc >= 2) for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  valid_sym(Sym);
  if (argc == 2) {
      t = time(NULL);
      if ((TM = localtime(&t)) == NULL) {
	perror("localtime");
	exit(EXIT_FAILURE);
      }
      if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
	fprintf(stderr, "strftime returned 0");
	exit(EXIT_FAILURE);
      }
  }
  if (argc == 3) {
    if (strlen(argv[2]) == 10) {	// process a date
      strcpy(qDate, argv[2]);
    } else {
      Safe_Periods=atoi(argv[2]);	// save Safe_Periods
      t = time(NULL);	// use today for qDate
      TM = localtime(&t);
      if (TM == NULL) {
	perror("localtime");
	exit(EXIT_FAILURE);
      }
      if (strftime(qDate, sizeof(qDate), "%F", TM) == 0) {
	fprintf(stderr, "strftime returned 0");
	exit(EXIT_FAILURE);
      }
    }
  }
  valid_date(Sym);
  sprintf(query,"select day_high,day_low from stockprices where symbol = \"%s\" order by date asc",Sym);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);	  // save the query results
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol \"%s\"\n", Sym);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  
  if (num_rows < (Safe_Periods+Safe_Stickyness+Safe_Coeff)) {
    printf("Not enough rows found to process SafeZone\n");
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  // find last_sticky_high/low, StartRow is (qDate-1)-Safe_Stickyness
  StartRow = num_rows-1-Safe_Stickyness;
  mysql_data_seek(result, StartRow);
  last_sticky_low = last_sticky_high = StartRow;
  for(x=StartRow; x<num_rows-1; x++) {
    row=mysql_fetch_row(result);
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    // check for nulls
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
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
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    // check for nulls
    lengths=mysql_fetch_lengths(result);
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    tval=strtof(row[1],NULL);
    row=mysql_fetch_row(result);
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    // check for nulls
    lengths=mysql_fetch_lengths(result);
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
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
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }

  for(x=StartRow; x<last_sticky_low; x++) {
    tval=strtof(row[0],NULL);
    row=mysql_fetch_row(result);
    // check for nulls
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
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
    if (row==NULL) {	// no results returned
      if (mysql_errno(mysql)) {	// an error was reported
	print_error(mysql, "fetch_row failed");
      }
    }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"Null data found, abandoning process"); }
    safeup_val = fmax(safeup_val, strtof(row[1],NULL)-(avg_up*(float)Safe_Coeff));
    safedn_val = fmin(safedn_val, strtof(row[0],NULL)+(avg_dn*(float)Safe_Coeff));
  }
  printf("[%s] =\t%.4f\t%.4f\n", qDate,safeup_val,safedn_val);

  // finished with the database
  #include "../Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
