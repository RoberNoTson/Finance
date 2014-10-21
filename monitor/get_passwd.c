/* get_passwd.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"
int	get_passwd(char * password) {
  struct	termios oflags, nflags;

  // disable echo
  tcgetattr(fileno(stdin), &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;
  if (tcsetattr(fileno(stdin) ,TCSANOW, &nflags) != 0) {
    puts("Error setting up STDIN for password prompt");
    Cleanup(EXIT_FAILURE);
  }
  printf("Enter password for db: ");
  fgets(password, sizeof(password),stdin);
  password[strlen(password)-1] = 0;
  // restore terminal echo
  if (tcsetattr(fileno(stdin) ,TCSANOW, &oflags) != 0) {
    puts("Error setting up STDIN for password prompt");
    Cleanup(EXIT_FAILURE);
  }
  return(EXIT_SUCCESS);
}