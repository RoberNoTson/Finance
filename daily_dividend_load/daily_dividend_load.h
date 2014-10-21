// daily_dividend_load.h
#ifndef	DAILY_DIV_LOAD_H
#define	DAILY_DIV_LOAD_H 1
#define	DAILY_DIV_LOAD_VERSION 0.1

#define		DEBUG	0
#define		MINVOLUME	"80000"
#define		MINPRICE	"14"
#define		MAXPRICE	"250"
#define		_XOPEN_SOURCE	1
#define		MAX_PERIODS	200

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>
#include	<ctype.h>
#include	<curl/curl.h>

struct	MemStruct {
  char *memory;
  size_t size;
};
struct	MemStruct	chunk;

MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
char	qDate[12];

/* external functions */
  extern	int xlate_date(char *);

#endif
