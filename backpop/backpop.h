#ifndef BACKPOP_H
#define BACKPOP_H 1
#define BACKPOP_VERSION 0.1

// TEST set DEBUG to 1, else set to 0 for PROD
#define		DEBUG	0
#define		_XOPENSOURCE
#define		_XOPEN_SOURCE
#define		DAY_SECONDS     3600*24

#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>
#include	<ctype.h>

char    errbuf[CURL_ERROR_SIZE];
CURL *curl;
CURLcode        res;
struct  MemStruct chunk;
MYSQL *mysql;
struct  MemStruct {
  char *memory;
  size_t size;
} chunk;

MYSQL_RES *result;
MYSQL_ROW row;

/* external variables */
  extern	time_t	t,t2;
  extern	struct tm *TM, *TM2;
  extern	int	updated;
  
/* external functions */
  extern        void     Usage(char *);
  extern        size_t  ParseData(void *, size_t, size_t, void *);
  extern	int	get_history(char *, char *);
  
#endif  // BACKPOP_H
