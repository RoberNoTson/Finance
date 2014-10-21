/* xlate_date.c
 * part of daily_dividend_load
 * translate various date formats to yyyy-mm-dd
 */
#ifndef	_XLATE_DATE
#define	_XLATE_DATE 1
#include	"./daily_dividend_load.h"

int	xlate_date(char *inDate) {
  char	buf[1024];
  char	buf2[1024];
  char	*saveptr;
  time_t t;
  struct tm *TM;
  char	CurYear[6];

  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    if (mysql != NULL) mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  strftime(CurYear,sizeof(CurYear),"%Y",TM);

  memset(buf,0,sizeof(buf));
  memset(buf2,0,sizeof(buf2));
  strcpy(buf,inDate);
  
  if ((saveptr=strstr(buf,"-"))) {	// parse the year
    strcpy(inDate,strstr(saveptr+1,"-")+1);
  } else if ((saveptr=strchr(buf,','))) {
    strncpy(inDate,saveptr+2,4);
    memset(inDate+4,0,1);
    memset(saveptr,0,1);
  } else {
    strcpy(inDate,CurYear);
  }
  if (strlen(inDate) == 2) {
    strncpy(buf2,inDate,2);
    sprintf(inDate,"20%s",buf2);
  }
  
  if (strcasestr(buf,"JAN")) strcat(inDate,"-01-");
  else if (strcasestr(buf,"FEB")) strcat(inDate,"-02-");
  else if (strcasestr(buf,"MAR")) strcat(inDate,"-03-");
  else if (strcasestr(buf,"APR")) strcat(inDate,"-04-");
  else if (strcasestr(buf,"MAY")) strcat(inDate,"-05-");
  else if (strcasestr(buf,"JUN")) strcat(inDate,"-06-");
  else if (strcasestr(buf,"JUL")) strcat(inDate,"-07-");
  else if (strcasestr(buf,"AUG")) strcat(inDate,"-08-");
  else if (strcasestr(buf,"SEP")) strcat(inDate,"-09-");
  else if (strcasestr(buf,"OCT")) strcat(inDate,"-10-");
  else if (strcasestr(buf,"NOV")) strcat(inDate,"-11-");
  else if (strcasestr(buf,"DEC")) strcat(inDate,"-12-");
  else return(EXIT_FAILURE);
  if ((saveptr=strstr(buf,"-"))!=NULL) {	// parse the day
    memset(saveptr,0,1);
    if (strstr(buf," ")) {
    if (strlen(buf+1) == 1) strcat(inDate,"0");
      strcat(inDate,buf+1);
    } else strcat(inDate,buf);
  } else {
    saveptr=rindex(buf,' ');
    if (strlen(saveptr+1) == 1) strcat(inDate,"0");
    strcat(inDate,saveptr+1);
  }
  return(EXIT_SUCCESS);
}
#endif	// _XLATE_DATE
