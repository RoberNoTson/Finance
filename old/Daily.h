// Daily.h
  FILE	*daily_txt;
  char	*filename="/Finance/Investments/Daily.list";
  char	*perms="w+";
  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NasdaqNM\",\"NGM\", \"NCM\", \"NYSE\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  char	query[200];
  char	*qURL1="http://download.finance.yahoo.com/d/quotes.csv?s=";
  char	*qURL2="&e=.csv&f=b3b2";
  char	qURL[strlen(qURL1)+strlen(qURL2)+(argc>1 ? strlen(argv[1]) : 1)+1];
  char	Bid[10],Ask[10];
  char 	*saveptr;
  CURL *curl;
  CURLcode	res;
  struct	MemStruct	chunk;
  MYSQL_RES *result_list;
  MYSQL_ROW row_list;
  MYSQL_FIELD *field;
  time_t t;
  struct tm *TM;
  unsigned long	*lengths;
  float	CurHigh,PrevHigh,Prev2High,CurLow,PrevLow,Prev2Low,CurClose,PrevClose,Prev2Close,PrevOpen,Prev2Open;
  float Max,Min=0,tval,ChanUp=0,ChanDn=0;
  float	ATR=0;
  float A,B,C,TR,PP,R1,R2,S1,S2;
  float	Min_val=FLT_MAX;
  float	Max_val=0;
  float	sum_up=0;
  float	sum_dn=0;
  float	safeup_val=0;
  float	safedn_val=FLT_MAX;
  float	avg_up,avg_dn;
  int	Coeff=3;
  int	ATR_Periods=22;
  int	MinMax_Periods=22;
  int	StartRow;
  int	num_rows;
  int	Safe_Periods=20;
  int	Safe_Coeff=2;
  int	Safe_Stickyness=6;
  int	num_low_diff=0;
  int	num_high_diff=0;
  int	x,last_sticky_high,last_sticky_low;
