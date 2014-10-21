/* get_input.c
 * part of sell_stock
 */
#include	"sell_stock.h"

int	get_input(void) {
  char	query[1024];
  int	x;
  
 // get number of shares sold - default smaller of initial buy or current holdings
  if (DEBUG) printf("buy_shares %d\tholdings %d\n",buy_shares,holdings);
//  printf("Number of shares sold [%d]: ",buy_shares<holdings?buy_shares:holdings);
  printf("Number of shares sold [%d]: ",Lot_size);
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) {
//    Shares = buy_shares<=updated_holdings ? buy_shares : updated_holdings;
    Shares = Lot_size;
  } else {
    errno=0;
    Shares=strtol(query,NULL,10);
    if (errno) { puts("Shares conversion error"); return(EXIT_FAILURE); }
  }
  if (DEBUG) printf("Shares: %d\n",Shares);
  // invalid attempt, too many shares sold
  if (Shares>holdings) {
    puts("Trying to sell more shares than available, giving up");
    return(EXIT_FAILURE);
  }
  
  // TBD - change this to allow selection of more holdings
  if (Shares>buy_shares) {
    puts("Trying to sell more shares than were purchased in this Lot.\nPlease split the sale and try again.");
    return(EXIT_FAILURE);
  }
  
// get price of shares sold - no default
  printf("Price of shares sold: ");
  errno=0;
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) { puts("No price entered"); return(EXIT_FAILURE); }
  if ((x=sscanf(query,"%f",&Price)) == 0) { puts("Invalid price"); return(EXIT_FAILURE); }
  else if (errno)  { puts("Read error\n"); return(EXIT_FAILURE); }
  else if (x == EOF) { puts("No identifiable selection made"); return(EXIT_FAILURE); }
  if (DEBUG) printf("Price: %.4f\n",Price);
  
// get sale date (default today)
//  printf("Sale date [%s]: ",prevDate);
  printf("Sale date [%s]: ",qDate);
  fgets(sDate,sizeof(sDate),stdin);
  if (strchr(sDate,'\n')) memset(strchr(sDate,'\n'),0,1);
//  if (!strlen(sDate)) strcpy(sDate,prevDate);
  if (!strlen(sDate)) strcpy(sDate,qDate);
  if (DEBUG) printf("Date: %s\n",sDate);
  
// get sale commission/fees - default 0.00
  printf("Broker commision [0.00]: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) Comm=0.00;
  else Comm=strtof(query,NULL);
  if (DEBUG) printf("Comm: %.2f\n",Comm);
  printf("Sale fees [0.00]: ");
  fgets(query,sizeof(query),stdin);
  if (strchr(query,'\n')) memset(strchr(query,'\n'),0,1);
  if (!strlen(query)) Fees=0.00;
  else Fees=strtof(query,NULL);
  if (DEBUG) printf("Fees: %.2f\n",Fees);
  // last verification before commit
  printf("All values correct? [y/n]: ");
  query[0] = getchar();
  if (strncmp(query,"y",1)) { puts("No changes made"); return(EXIT_FAILURE); }
  return(EXIT_SUCCESS);
}
