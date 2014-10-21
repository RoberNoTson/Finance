// my-trades.h
#ifndef MY_TRADES_H
#define MY_TRADES_H 1
#define MY_TRADES_VERSION 0.2

#define DEBUG   0
#define _XOPENSOURCE
#define		MAX_PERIODS	200
#define		MINVOLUME "400000"
#define		MINPRICE "7"
#define		MAXPRICE "250"

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>
#include	<error.h>
#include	<curl/curl.h>
#include	<sys/types.h>
#include	<sys/wait.h>

struct	MemStruct {
  char *memory;
  size_t size;
};
struct	MemStruct	chunk;

MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];
double	SafeUp,SafeDn;
double	ChanUp,ChanDn;
double	Bid,Ask;

/* external variables */
  extern	double	ChanUp,ChanDn;
  extern	double	SafeUp,SafeDn;

/* external functions */
  extern	int	Usage(char *);
  extern	size_t  ParseData(void *, size_t, size_t, void *);
  extern	int	delete_bad(char *);
  extern	int	BullishOB(char *);
  extern	int	BullishKR(char *);
  extern	int	TrendUp(char *);
  extern	float	RSI_check(char *);
  extern	int	do_output(char *);
  extern	int	MACD_check(char *);
  extern	int	HV_check(char *);
  
#endif	// MY_TRADES_H
  