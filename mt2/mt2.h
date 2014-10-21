// mt2.h
#ifndef	MT2_H
#define	MT2_H 1
#define	MT2_VERSION 0.1

#define	DEBUG	0	// use 0/blank for prod, or 1 for test
#define	RELEASE	1	// use 1 for "prod", or 0 for "test"

//#if RELEASE == 1
#define	DB_INVESTMENTS	"Investments"	// use "test_invest" or "Investments"
#define	DB_BEANCOUNTER	"beancounter"	// use "test_bean" or "beancounter"
//#else
//#define	DB_INVESTMENTS	"test_invest"	// use "test_invest" or "Investments"
//#define	DB_BEANCOUNTER	"test_bean"	// use "test_bean" or "beancounter"
//#endif

#define		_XOPEN_SOURCE
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

MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;

/* external variables */
  extern	char	qDate[12];
  extern	char	priorDate[12];
  extern	char	prevpriorDate[12];

/* external functions */
  extern	int     Usage(char *);
  extern	int	do_output(void);
  extern	int	do_updates(void);
  
#endif	// MT2_H
