// QuoteOptions.h
#ifndef	QUOTE_OPTIONS_H
#define	QUOTE_OPTIONS_H 1
#define	QUOTE_OPTIONS_version 0.1

#define		DEBUG	1
#define		_XOPENSOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<curl/curl.h>
#include	<ctype.h>


struct	MemStruct {
  char *memory;
  size_t size;
};
struct	MemStruct chunk;

char	errbuf[CURL_ERROR_SIZE];
CURL *curl;
CURLcode	res;

  struct OPTION {
  char	symbol[32];	// format SYMYYMMDDtNNNNNNNN e.g. IBM120519P00260000
  char	ticker[12];	// stock ticker e.g. IBM
  float	strike;
  float	bid;
  float	ask;
  float	last;
  float	open;
  float	change;
  int		volume;
  int		in_the_money;
  };
  struct OPTION	*option;
  
  struct	EXPIRATION {
  char		ticker[16];
  char		exp[12];	// in YYYYMMDD format
  int	num_puts;
  int	num_calls;
  struct OPTION	*calls;
  struct OPTION	*puts;
  };
  struct	EXPIRATION	*expiration;
  struct	EXPIRATION	*pExpiration;

/* external variables */
  char	cur_exp_date[16];
  char	query[1024];
  char	*saveptr;
  char	*endptr;
  char	*future_exp_dates;
  int	num_exp;

/* external functions */
  extern	int get_exp_dates(char *);
  
#endif	// QUOTE_OPTIONS_H
