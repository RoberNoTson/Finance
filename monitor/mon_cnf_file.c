/* mon_cnf_file.c
* part of watchlist_monitor
* uses inotify to monitor the config file for changes, reloads it when changed
* uses kill(child_pid,SIGUSR1) to signal the reload.
*/

#include "watchlist_monitor.h"
#include <sys/inotify.h>
#include <sys/select.h>

#define	EVENT_SIZE	(sizeof(struct inotify_event))
#define	EVENT_BUF_LEN	( 1024 * (EVENT_SIZE+16))


int monitor_config_file(pid_t child_pid) {
  int	inotify_fd;
  int	watch_fd;
  int	length=0;
  char	in_buf[EVENT_BUF_LEN];
  struct	stat FileInfo;
  struct inotify_event *event = ( struct inotify_event * ) &in_buf; 

if (debug) puts("#### starting monitor_config_file thread ####");
  if ((inotify_fd=inotify_init() )<0) {
    // error 
    puts("inotify_init failed!");
    return(EXIT_FAILURE);
  }
  watch_fd=inotify_add_watch(inotify_fd,config_file_name,IN_ATTRIB);
  for(;;) {
    if ((length=read(inotify_fd,in_buf,EVENT_BUF_LEN)) < 0) {
      puts("inotify read failed!");
      return(EXIT_FAILURE);
    }
    if (debug) printf("#### mon_cnf_file has read %d bytes ####\n",length);
    if (event->mask & IN_ATTRIB) {
      if (debug) puts("#### signaling reload of config file ####");
//      kill(child_pid,SIGUSR1);
      if ((stat(config_file_name, &FileInfo)<0) || (ParseConfig(prog)>0)) {
	printf("Error reading config file %s\n",config_file_name);
	return(EXIT_FAILURE);
      }
      printf ("Reloaded config file\n");
    }
  }	// end FOR
  close(inotify_fd);
  return(EXIT_SUCCESS);
}

