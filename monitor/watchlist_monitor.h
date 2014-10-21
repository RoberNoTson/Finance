// watchlist_monitor.h
#ifndef WATCHLIST_MONITOR_H
#define WATCHLIST_MONITOR_H 1
#define MONITOR_VERSION 0.1

#define	DEBUG	0	// 1/true for testing, 0/false for production
#define	RELEASE	1	// 0=test db, 1=prod db

#define	DEFAULT_INTERVAL	60
#define	MAXPARA 10	// normal value is 5
#define	STOPLOSS_PERCENT	0.05

#define         _XOPENSOURCE

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <unistd.h>
#include        <curl/curl.h>
#include	<termios.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<pthread.h>

struct  MemStruct {
  char *memory;
  size_t size;
};

struct stocks {
  char SYMBOL[12];
  char date[12];
  float MEDIAN_BUY;
  float MEDIAN_SELL;
  float AVG_SELL;
  float MEDIAN_SELL_NEXT;
  float AVG_SELL_NEXT;
  float PAPER_BUY_PRICE;
  float PAPER_SELL_PRICE;
  float STOP_PRICE;
}; 
struct stocks *Stocks;

char	*stock_table;
char	*holdings_table;
char	db_pass[128];
char	refresh_interval[16];

/* shared variables */
  extern	char  Sym[];
  extern	struct        MemStruct chunk;
  extern	char    config_file_name[];
  extern	int	debug,verbose,email,interval,do_output_lock;
  extern	int	log_notices;
  extern	char	config_file_name[];
  extern	char	email_addr_1[];
  extern	char	email_addr_2[];
  extern	char	log_file[];
  extern	char	Sym[];
  extern	char	database[];
  extern	char	db_host[];
  extern	char	db_user[];
  extern	char	db_pass[];
  extern	char	market_open[];
  extern	char	market_close[];
  extern	char	refresh_interval[];
  extern	char	buyDate[];
  extern	char	sellDate[];
  extern	char	query[];
  extern	char	prog[];
  extern	struct sigaction sig_act, sig_oldact;
  extern	struct	termios	current_term;
  extern	struct	termios  save_term;
  extern	char	*stock_table;

/* external functions */
  extern	int	ParseConfig (char *);
  extern	int	FreeAllMem (void);
  extern	void	Usage (char *);
  extern	size_t	ParseData(void *, size_t, size_t, void *);
  extern	void	Cleanup(int);
  extern	int	SigHandler(int);
  extern	int	time_check(void);
  extern	int	build_scan_table(void);
  extern	int	scan_db(void);
  extern	float	get_RTquote(char *);
  extern	int	scan_watchlist(int);
  extern	int	scan_watchlist_parse(int);
  extern	int	scan_holdings(void);
  extern	int	scan_holdings_parse(int);
  extern	int	do_output(char *);
  extern	int	monitor_config_file(pid_t);
  
#endif // WATCHLIST_MONITOR_H
