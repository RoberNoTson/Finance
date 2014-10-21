/* sell_stock.h
 * part of sell_stock
 */
#ifndef	SELL_STOCK_H
#define	SELL_STOCK_H 1
#define	SELL_STOCK_VERSION	0.1

#define	DEBUG	0
#define		_XOPEN_SOURCE

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<errno.h>

/* shared variables */
MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];
char	prevDate[12];


/* external variables */
  extern	char	Sym[16];
  extern	char	buy_date[12];
  extern	char	sDate[12];
  extern	int	holdings;
  extern	int	updated_holdings;
  extern	int	buy_shares;
  extern	int	ID;
  extern	int	Shares;
  extern	int	buy_shares;
  extern	int	Lot_size;
  extern	float	buy_price;
  extern	float	Price;
  extern	float	Comm;
  extern	float	Fees;

/* external functions */
  extern	int	create_menu(void);
  extern	int	get_input(void);
  extern	int	update_beancounter_db(void);
  extern	int	update_Investments_db(void);
  extern	int	update_watchlist(void);
  
#endif	// SELL_STOCK_H
