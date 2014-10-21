// daily_div_list.h
#ifndef	LIST_DIVS_DAILY_H
#define	LIST_DIVS_DAILY_H 1
#define	LIST_DIVS_DAILY_H_VERSION 0.1

#define		DEBUG	0		// set to 0 or 1 to disable/enable debug code
#define		SPAN		28	// number of days ahead to look, usually 21
#define		MINYIELD	"1.5"
#define		MINVAL		"1.00"
#define		MINPRICE	"10.0"
#define		MINVOLUME	"80000"
#define		MAX_PERIODS	200
#define		DAY_SECONDS	3600*24
#define 	_XOPENSOURCE
#define		OUT_FILE "/Finance/Investments/daily_div_list.txt"

#include	<my_global.h>
#include	<my_sys.h>
#include	<mysql.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>
#include	<curl/curl.h>

static MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;

struct	MemStruct {
  char *memory;
  size_t size;
};

double  SafeUp,SafeDn;
double  ChanUp,ChanDn;
double  Bid,Ask;
double	PP,R1,R2,S1,S2;
char	qDate[12];

/* external variables */
  extern        time_t  t,t2;
  extern        struct tm *TM, *TM2;
  extern        int     updated;
  
/* external functions */
  extern        void     Usage(char *);
  extern        size_t  ParseData(void *, size_t, size_t, void *);
  extern        int     get_history(char *, char *);

#endif	// LIST_DIVS_DAILY_H
