/* watchlist_monitor.c
 * monitor stock symbols from the database for buy/sell prices
 * SIGINT ends the program
 * SIGUSR1 reloads the config file
 * 
 * Parms: [config file]
 * compile: N/A - use make [Makefile]
 */

#include "watchlist_monitor.h"

int	debug=0;
int	verbose=0,email=0;
int	log_notices=0;
int	interval=60;
int	do_output_lock=0;
char	config_file_name[1024];
char	email_addr_1[1024];
char	email_addr_2[1024];
char	log_file[1024];
char	Sym[16];
char	market_open[16];
char	market_close[16];
char	buyDate[12];
char	sellDate[12];
char	query[1024];
char	prog[1024];
CURL *curl = 0;
struct        MemStruct chunk;
MYSQL *mysql;
MYSQL_RES *result;
MYSQL_ROW row;
pid_t	child_pid=0,mon_pid=0;
struct sigaction sig_act, sig_oldact;
struct	termios	current_term;
struct	termios  save_term;

#include	"../Includes/print_error.inc"

int main (int argc, char *argv[]) {
  struct	stat FileInfo;

  // parse CLI parms, if any
  strcpy(prog,basename(argv[0]));
  if (argc > 2) Usage(prog);
   // is there a config file name?
  if (argc == 2) {
     if (strstr(argv[1],"-")==0) {
	strcpy(config_file_name, argv[1]);
     } else Usage(prog);
  } else {	// argc == 1
     // parse $HOME and setup default config file path
     memset(config_file_name,0,sizeof(config_file_name));
     sprintf(config_file_name,"%s/%s.cnf",getenv("HOME"),prog);
  }
  if ((stat(config_file_name, &FileInfo)<0) || (ParseConfig(prog)>0)) {
    printf("Error reading config file \"%s\"\n",config_file_name);
    exit(EXIT_FAILURE);
  }
  // initialize systems
  chunk.memory=0;
  mysql=0;
  do_output_lock=0;
  
  // fork subprocess to loop through data
  errno = 0;
  child_pid = fork();
  if (child_pid == -1) {
    perror("fork attempt failed...");
    exit(EXIT_FAILURE);
  }
  if (child_pid==0) {	// fork to actual work process
    // set SIGNAl handler pointers
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_handler=SigHandler;
    sigaction (SIGINT, &sig_act, NULL);
    sigaction (SIGUSR1, &sig_act, NULL);
    sigaction (SIGUSR2, &sig_act, NULL);
    // do the real work
    scan_db();
    raise(SIGINT);
    Cleanup(EXIT_SUCCESS);
  } 
  if (debug) printf("#### scan_db running as PID %d ####\n",child_pid);
  
  // monitor the configuration file for changes using inotify(), automatically reload it as needed using SIGUSR1
  errno = 0;
  mon_pid = fork();
  if (mon_pid == -1) {
    perror("monitor_cfg_file fork attempt failed...");
    exit(EXIT_FAILURE);
  }
  if (mon_pid==0) { 
    monitor_config_file(child_pid);
    exit(EXIT_SUCCESS);
  }
  if (debug) printf("#### monitor_config_file running as PID %d ####\n",mon_pid);
  
  // local process to watch for ^C, or other signals
  // Set terminal to read chars immediately w/o echo
    tcgetattr(0, &save_term);
    current_term = save_term;
    current_term.c_lflag &= ~ICANON;
    current_term.c_lflag &= ~ECHO;
    current_term.c_cc[VMIN] = 1;
    current_term.c_cc[VTIME]	= 0;
    tcsetattr(0,TCSADRAIN,&current_term);
  // monitor keyboard input for options
    for (;;) {
      int t = getchar();
      switch(t) {
	case 'h': case 'H': case '?':
	   printf("Commands:\n\tq|Q\tQuit\n\tr|R\tReload Config\n\tf|F\tForce check\n\th|H|?\tHelp\n");
	   break;
	case 'q': case 'Q':
	   puts("Terminating");
	   kill(child_pid,SIGINT);
	   wait(NULL);
           tcsetattr(0,TCSADRAIN,&save_term);
	   exit(0);
	   break;
	case 'r': case 'R':
	   puts("Sending Reload signal");
	   kill(child_pid, SIGUSR1);
	   break;
	case 'f': case 'F':
	  printf("Checking prices...");
	  // run online scan
	  puts("done");
	  break;
	default:
	   break;
      } // end switch
    }  // end for
    Cleanup(EXIT_SUCCESS);
  // we will never actually reach this point, since one of the threads will exit or catch ^C (SIGINT).
  // but the compiler gripes if this isn't here
  exit(EXIT_SUCCESS);
}	// end Main
