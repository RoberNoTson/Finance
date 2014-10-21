/* insert_Dividend.c
 * update beancounter.cash by adding a cash dividend value
 * 
 * Parms: [0.00] (will prompt if needed)
 * compile: gcc -Wall -O2 -ffast-math insert_Dividend.c -o insert_Dividend `mysql_config --include --libs`
 */

#define	DEBUG	0
#define		_XOPEN_SOURCE

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<errno.h>

MYSQL *mysql;

#include        "Includes/print_error.inc"

int	main (int argc, char *argv[]) {
  char	qDate[12];
  char	div_date[12];
  char	Sym[16];
  char	query[1024];
  int	x;
  float	value=0.0;
  time_t t;
  struct tm *TM = 0;

  memset(Sym,0,sizeof(Sym));
  if (argc==2) { for (x=0;x<strlen(argv[1]);x++) Sym[x]=toupper(argv[1][x]); }
  memset(qDate,0,sizeof(qDate));
    // initialize the time structures with today
  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  strftime(qDate, sizeof(qDate), "%F", TM);
  if (argc > 2) {
    printf("Usage: %s [ [+]0.00]\n",argv[0]);
    puts("Pass the dollar value to add, or it will prompt for it.");
    puts("The plus sign \"+\" is optional, dividends will always add the amount");
    exit(EXIT_FAILURE);
  }
  if (argc == 2) value = strtof(argv[1],NULL);
  if (argc == 1) {	// prompt for the cash value to add
    printf("Dividend Amount: ");
    fgets(query,sizeof(query),stdin);
    if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
    if (!strlen(query)) value=0.00;
    else value = fabsf(strtof(query,NULL));
    if (DEBUG) printf("Amt: %.2f\n",value);
    if (value == 0) {
      puts("No value found, exiting unchanged");
      exit(EXIT_FAILURE);
    }
  }
  printf("Dividend pay date [%s]: ",qDate);
  fgets(div_date,sizeof(div_date),stdin);
  if (strchr(div_date,'\n')) memset(strchr(div_date,'\n'),0,1);
  if (!strlen(div_date)) strcpy(div_date,qDate);
  if (DEBUG) printf("Date: %s\n",div_date);

  // last verification before commit
//  if (value < 0) value *= -1;
  printf("Verify this:  Add $%.2f Dividend on %s\n",value,div_date);
  printf("All values correct? [y/N]");
  query[0] = getchar();
  if (strncmp(query,"y",1)) {
    puts("No changes made");
    exit(EXIT_FAILURE);
  }
  #include	"Includes/beancounter-conn.inc"
  sprintf(query,"insert into beancounter.cash () values(\"ML-CMA\",\"%.2f\",DEFAULT,\"Dividend\",DEFAULT,DEFAULT,\"%s\")",
	  value,div_date);
  if (DEBUG) printf("%s\n",query);
  if (!DEBUG) if(mysql_query(mysql,query)) print_error(mysql,"Failed to update database");
  
  puts("Database updated with new Dividend cash value");
  // finished with the database
  #include "Includes/mysql-disconn.inc"
  exit(EXIT_SUCCESS);
}
