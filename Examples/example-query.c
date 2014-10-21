// Basic MySQL query program using C interface 
// gcc -O2 -o example-query example-query.c `mysql_config --include --libs`
// if compile has strange errors, try this format
// CFG=/usr/bin/mysql_config; sh -c "gcc -o example-query `$CFG --cflags` example-query.c `$CFG --libs`"

#define	QUERY1 "select day_low from stockprices where symbol='IBM' order by date"

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	"print_error.inc"

static MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;

int	main(int argc, char *argv) {
  int	num_rows,num_fields,i;
  unsigned long *lengths;
  
// set up the database connection
#include "beancounter-conn.c"
  
// connected now, here begins the query processing
  if (mysql_query(mysql,QUERY1)) {
//    fprintf(stderr, "Failed to query database: Error %s\n", mysql_error(mysql));
    print_error(mysql,"Failed to query database");
    return(1);
  }
  // save the query results
  result=mysql_store_result(mysql);
  if (result==NULL) {		// no results returned, might be okay if query was not a 'select'
    if (mysql_errno(mysql)) {		// any errors returned?
      fprintf(stderr, "Store results or query failed: Error %s\n", mysql_error(mysql));
      print_error(mysql,"Store results or query failed");
      return(1);
    }
  } 
  // get number of rows returned
  num_rows=mysql_num_rows(result);
  mysql_data_seek(result, num_rows-1);
  row=mysql_fetch_row(result);
  printf("%d rows in set, %s is value from row %d\n",num_rows,row[0],num_rows-1);
  // get number of columns in the row
  num_fields=mysql_num_fields(result);  
  
/*  // print the results
  while ((row=mysql_fetch_row(result))) {
//    lengths = mysql_fetch_lengths(result);  // only needed for binary data
    for(i=0;i<num_fields;i++) {
//      printf("%.*s",(int) lengths[i],row[i] ? row[i] : "NULL");	// lengths[] is only needed for binary data 
      printf("%s",row[i] ? row[i] : "NULL");	// ascii data will be null-terminated; NULL indicates an error.
    }
    printf("\n");
  }
*/  
  // query processing completed, free the result set
  mysql_free_result(result);
// ready for another query.

// finished with the database
#include "mysql-disconn.c"
  return(0);
}