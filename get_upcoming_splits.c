/* get_upcoming_splits.c
 * download list of upcoming stock splits from "http://biz.yahoo.com/c/s.html"
 * email notice on the ExecDate
 * ExecDate is the effective date of the new price, 
 * so the split starts at COB of the previous trading session, the ExSplit or HoldDate
 * 
 * Parms: none
 * compile: gcc -Wall -O2 -ffast-math get_upcoming_splits.c -o get_upcoming_splits `curl-config --libs`
 */

#define		DEBUG	0
#define		_XOPEN_SOURCE
#include	<stdlib.h>
#include	<stdio.h>
#include        <string.h>
#include	<unistd.h>
#include	<time.h>
#include	<curl/curl.h>

struct	MemStruct {
  char *memory;
  size_t size;
};

#include	"Includes/ParseData.inc"

int main(int argc, char * argv[]) {
  char	errbuf[CURL_ERROR_SIZE];
  char	HoldDate[12],ExecDate[12],Name[32],Symbol[12],Num[3],Denom[3];
  char	buf[32];
  char	CurDate[12];
  char	*saveptr;
  char	*endptr;
  int	x;
  time_t	t,cur_t;
  struct tm	*TM;
  CURL *curl;
  CURLcode	res;
  struct	MemStruct chunk;

  cur_t = time(NULL);
  TM = localtime(&cur_t);
  strftime(CurDate,sizeof(CurDate),"%F",TM);
  memset(TM,0,sizeof(TM));	// zero the hours, minutes,seconds, etc.
  strptime(CurDate,"%F",TM);
  cur_t = mktime(TM);
  chunk.memory = calloc(1,1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_NOTHING);
  curl=curl_easy_init();
  if(!curl) {
    puts("curl init failed, aborting process");
    exit(EXIT_FAILURE);
  }    
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParseData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_URL,"http://biz.yahoo.com/c/s.html");
  res=curl_easy_perform(curl);
  if (res) {	// 0 means no errors
      if (res != 56) {
	printf("Error %d Unable to access the internet\n",res);
	printf("%s\n",errbuf);
	exit(EXIT_FAILURE);
      } else {	// retry
	sleep(5);
	res=curl_easy_perform(curl);
	if (res) {
	  printf("Error %d Unable to access the internet\n",res);
	  printf("%s\n",errbuf);
	  exit(EXIT_FAILURE);
	}
      }
  }
  // find "Payable"
  if ((saveptr = strstr(chunk.memory,"Payable")) == NULL) {
    puts("01 Error parsing incomplete or garbled data, exiting");
    exit(EXIT_FAILURE);
  }
  // find end of data
  if ((endptr = strstr(saveptr,"</table>")) == NULL) {
    puts("02 Error parsing incomplete or garbled data, exiting");
    exit(EXIT_FAILURE);
  }
  memset(HoldDate,0,sizeof(HoldDate));
  memset(ExecDate,0,sizeof(ExecDate));
  memset(Name,0,sizeof(Name));
  memset(Symbol,0,sizeof(Symbol));
  memset(Num,0,sizeof(Num));
  memset(Denom,0,sizeof(Denom));
  memset(buf,0,sizeof(buf));
  puts("Symbol\tRatio\tExSplit\t\tEffective");
  
  // Loop through data
  while (saveptr < endptr) {
    if ((saveptr = strstr(saveptr,"<tr bgcolor=")) == NULL) break;	// no more data
    if ((x=sscanf(saveptr,"<tr bgcolor= %*[A-Za-z0-9 ]><td align=center> %[A-Za-z0-9&; ] </td>\
      <td align=center> %[A-Za-z0-9.&-; ] </td><td> %[A-Za-z0-9.&()/;-', ] </td>\
      <td align=center><a href=\"http://finance.yahoo.com/q?s= %*[a-z&.;] =t\"> %[A-Za-z.] </a></td>\
      <td align=center> %*[YN] </td><td align=center> %[0-9] - %[0-9] < %*s </tr>"
      ,buf,ExecDate,Name,Symbol,Num,Denom)) != 6) 
    {
      printf("sscanf failed on %s, %d matched\n",Name,x);
      printf("FAILED - HoldDate %s ExecDate %s Name %s Symbol %s Ratio %s/%s\n",buf,ExecDate,Name,Symbol,Num,Denom);
      exit(EXIT_FAILURE);
    }
//printf("Processed %s\n", Name);    
    saveptr++;
    // adjust the date format to match ISO standard
    memset(TM,0,sizeof(TM));
    if (!strcmp(buf,"&nbsp")) memset(buf,0,sizeof(buf));
    else {
      if (strptime(buf,"%b %d",TM)) strftime(HoldDate,sizeof(HoldDate),"%F",TM);
      else { puts("Failed to convert HoldDate, exiting"); exit(EXIT_FAILURE); }
    }
    if (strptime(ExecDate,"%b %d",TM)) strftime(ExecDate,sizeof(ExecDate),"%F",TM);
    else { puts("Failed to convert ExecDate, exiting"); exit(EXIT_FAILURE); }
    t = mktime(TM);
    if (t < cur_t) {
      if (DEBUG) puts("skipping invalid time");
      continue;
    }
    printf("%s\t%s/%s\t%s\t%s\n",Symbol,Num,Denom,HoldDate,ExecDate);
    if (t == cur_t) {
      sprintf(buf,"echo \"Symbol\tRatio\tExSplit\t\tEffective\n%s\t%s/%s\t%s\t%s\" | mailx -s \"Stock Splits\" mark_roberson@tx.rr.com",Symbol,Num,Denom,HoldDate,ExecDate);
      if (!DEBUG) system(buf);
    }
  }	// end While loop
  if (DEBUG) puts("end of While loop");
  
  curl_easy_cleanup(curl);
  if (DEBUG) puts("CURL cleanup okay");
//  free(chunk.memory);
  if (DEBUG) puts("mem free");
  exit(EXIT_SUCCESS);
}
