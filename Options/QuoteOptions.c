//QuoteOptions.c
/* emulate and improve on the Perl function Finance::QuoteOptions, which is broken
 * Main query page:
 * http://finance.yahoo.com/q/op?s=SYM
 * Additional expirations:
 * http://finance.yahoo.com/q/op?s=SYM&m=2012-06
 *
 * The main query page yields options for only the next expiration.
 * At the top of those tables is a list of other expiration months.
 * Generate the URLs for those additional pages and visit them
 * in turn to get all the options data.
 * 
 * Parms: Sym ["current"] - use the "current" parm to limit to the nearest expiration date puts/calls
 * compile: OBE use "make" gcc -Wall -O2 -ffast-math QuoteOptions.c -o QuoteOptions `mysql_config --include --libs` `curl-config --libs`
 */

#include	"QuoteOptions.h"

void	Usage(char *prog) {
  printf("Usage: %s Sym [\"current\"]\n",prog);
  puts("\"current\" flag limits results to the nearest expiration date only");
  exit(EXIT_FAILURE);
}

int	current=0;	// boolean "current" flag

int main(int argc, char * argv[]) {
  int	x;
  char	Sym[16];
  

  // parse CLI parms
  if (argc == 1) Usage(argv[0]);
  if (argc >= 2) for (x=0;x<strlen(argv[1]);x++) { Sym[x] = toupper(argv[1][x]); }
  if (argc == 3 && strcmp(argv[2],"current")) current++;

  get_exp_dates(Sym);
  

  
  expiration->calls = calloc(expiration->num_calls,sizeof(struct OPTION));
  expiration->puts = calloc(expiration->num_puts,sizeof(struct OPTION));
  option = expiration->calls;
  // do stuff
  
  
  option = expiration->puts;
  // do more stuff
  
  
  free(expiration->calls);
  free(expiration->puts);
  expiration++;
  // end of Loop
  
  
  
  
  

  curl_easy_cleanup(curl);
  free(chunk.memory);
  free(pExpiration);
  exit(EXIT_SUCCESS);
}	// end Main

