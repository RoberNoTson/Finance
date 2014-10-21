/* do_output.c
 * part of watchlist_monitor
 */

#include "watchlist_monitor.h"

int	do_output(char *msg) {
  time_t	t;
  struct tm *TM=0;
  struct timespec polltime;
  FILE	*l_file;
  char	mail_msg[1024];
  char	t_stamp[32];

  polltime.tv_sec = 0;
  polltime.tv_nsec = 100000;
//  while(do_output_lock) nanosleep(&polltime,NULL);
  do_output_lock++;
  if (strlen(msg)) {
    t=time(NULL);
    TM = localtime(&t);
    strftime(t_stamp,sizeof(t_stamp),"%F %T", TM);
    if (verbose) printf("%s %s\n",t_stamp,msg);
    if (log) {
	  l_file=fopen(log_file,"a+");
	  fprintf(l_file,"%s\t%s\n",t_stamp,msg);
	  fclose(l_file);
    }
    if (email) {
	  sprintf(mail_msg,"echo %s |mailx -s \"%s\" %s",msg,msg,email_addr_1);
	  system(mail_msg);
    }
  }
  do_output_lock--;
  return(EXIT_SUCCESS);
}

