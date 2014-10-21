// update_all_stocks.h
#ifndef UPDATE_ALL_STOCKS_H
#define UPDATE_ALL_STOCKS_H 1
#define  UPDATE_ALL_STOCKS_VERSION 0.1

#define DEBUG   0      // 1/true for testing, 0/false for production
#define RELEASE 1       // 0=test db, 1=prod db
#define MAXPARA 5       // normal value is 5

#define	_XOPENSOURCE
#define	_XOPEN_SOURCE
#define	DAY_SECONDS     3600*24

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include        <unistd.h>
#include        <curl/curl.h>
#include        <termios.h>
#include        <sys/types.h>
#include        <sys/wait.h>
#include        <pthread.h>

struct  MemStruct {
  char *memory;
  size_t size;
};

  char	qDate[12];

/* shared variables */
  extern        struct        MemStruct chunk;
  extern	struct tm *TM;
  extern	struct tm *TM2;
  extern	time_t	t,t2;
  extern        char	Sym[];
  extern        char    database[];
  extern        char    db_host[];
  extern        char    db_user[];
  extern        char    db_pass[];
  extern        char    market_open[];
  extern        char    market_close[];
  extern	char	qDate[];
  extern	int	mkt_open,debug;
  extern	int	force;

/* external functions */
  extern        void    Usage (char *);
  extern	int	time_check(void);
  extern	int	parse_update(char *);
  extern        size_t  ParseData(void *, size_t, size_t, void *);
  
#endif // UPDATE_ALL_STOCKS_H
