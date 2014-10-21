/* ParseConfig.c
 * part of watchlist_monitor
 */
#include "watchlist_monitor.h"
char	database[256];
char	db_host[128];
char	db_user[128];

int ParseConfig (char *prog) {
   int	LineNum = 0;
   char	line[80];
   char	InputBuffer[256] = {0};
   FILE   *config_file;
   struct	termios oflags, nflags;

// Load the configuration file, if it exists
  if (NULL == (config_file = fopen(config_file_name, "r"))) {
    printf ("Error opening configuration file %s\n", config_file_name);
    return(1);
  }
  if (debug) printf ("Using configuration file %s\r\n", config_file_name);
  // get configuration data from file
  memset(db_pass,0,sizeof(db_pass));
  memset(refresh_interval,0,sizeof(refresh_interval));
  while ( fgets (line, sizeof (line), config_file) != NULL) {
    ++LineNum;
    // skip over comments and blank lines
    if (line[0] == '#') continue;
    if (line[0] == ';') continue;
    if (line[0] == '\r') continue;
    if (line[0] == '\n') continue;
    if (sscanf (line, " debug%*[ =]%d",&debug) == 1) continue;
    if (sscanf (line, " verbose%*[ =]%d",&verbose) == 1) continue;
    if (sscanf (line, " log%*[ =]%d", &log_notices) == 1) continue;
    if (sscanf (line, " email%*[ =]%d", &email) == 1) continue;
    if (sscanf (line, " log_file%*[ =]%s", log_file) == 1) continue;
    if (sscanf (line, " email_addr_1%*[ =]%s", email_addr_1) == 1) continue;
    if (sscanf (line, " email_addr_2%*[ =]%s", email_addr_2) == 1) continue;
    if (sscanf (line, " market_open%*[ =]%s", market_open) == 1) continue;
    if (sscanf (line, " market_close%*[ =]%s", market_close) == 1) continue;
    if (sscanf (line, " interval%*[ =]%s", &refresh_interval) == 1) continue;
    if (sscanf (line, " database%*[ =]%s", database) == 1) continue;
    if (sscanf (line, " db_host%*[ =]%s", db_host) == 1) continue;
    if (sscanf (line, " db_user%*[ =]%s", db_user) == 1) continue;
    if (sscanf (line, " db_pass%*[ =]%s", db_pass) == 1) continue;
    if (sscanf (line, " db_pass%*[ =]") == 0) continue;
    // invalid entry in the file, return error
    printf("Error reading from configuration file %s\n",config_file_name);
    printf("Invalid parameter found in line %d\n", LineNum);
    return (1);
  } // end while
  fclose (config_file);
  
  // config file loaded, validate entries
  if (!verbose && !log && !email) {
    printf("No output formats are enabled. Please add at least one of \"verbose = 1\", \"log = 1\" or \"email = 1\" to the config file %s\n",config_file_name);
    Cleanup(EXIT_FAILURE);
  }
  if (email && !strlen(email_addr_1)) {
    printf("Email notification is enabled but no email address was found.\n Please add \"email_addr_1 = user@server\" to the config file %s\n",config_file_name);
    Cleanup(EXIT_FAILURE);
  }
  if (log && !strlen(log_file)) {
    sprintf(log_file,"%s/%s.log",getenv("HOME"),prog);
    printf("Using default log file %s\n",log_file);
  }
  
  if (!strlen(refresh_interval)) { 
    interval=DEFAULT_INTERVAL; 
    printf("Using default interval %d\n",interval);
  } else {
    // parse and convert refresh_interval to decimal value
    if (strchr(refresh_interval,'m') || strchr(refresh_interval,'M')) {
      interval = atoi(refresh_interval) * 60;
    } else {
      interval = atoi(refresh_interval);
    }
  }
  
  if (!strlen(market_open) || !strlen(market_close)) {
    printf("Market open/close times not found.\n Please ensure \"market_open = hh:mm \" and \"market_close = hh:mm\" are in the config file %s\n",config_file_name);
    Cleanup(EXIT_FAILURE);
  }
  // do some time validation/parsing stuff here to support different formats
  
  // database connection stuff
  if (!strlen(database)) {
    printf("No database name was found.\n Please add \"database = dbname\" to the config file %s\n",config_file_name);
    Cleanup(EXIT_FAILURE);
  }
  if (!strlen(db_host)) strcpy(db_host,"localhost");
  if (!strlen(db_user)) {
    printf("No database user name was found.\n Please add \"db_user = username\" to the config file %s\n",config_file_name);
    Cleanup(EXIT_FAILURE);
  }
  if (!strcmp(db_pass,"\"\"")) strcpy(db_pass," ");
  // prompt for database password
  if (!strlen(db_pass)) {
    if (current_term.c_cc[VMIN] == 1 && current_term.c_cc[VTIME] == 0)  tcsetattr(0,TCSADRAIN,&save_term);
    get_passwd(db_pass);
    if (current_term.c_cc[VMIN] == 1 && current_term.c_cc[VTIME] == 0)  tcsetattr(0,TCSADRAIN,&current_term);
    printf("\n");
  }

  if (debug) { 
    printf(" verbose = %d\n",verbose);
    printf(" log_file = %s\n", log_file);
    printf(" log = %d\n", log_notices);
    printf(" email = %d\n", email);
    printf(" email_addr_1 = %s\n", email_addr_1);
    printf(" email_addr_2 = %s\n", email_addr_2);
    printf(" market_open = %s\n", market_open);
    printf(" market_close = %s\n", market_close);
    printf(" interval = %s %d\n", refresh_interval,interval);
    printf(" db = %s\n", database);
    printf(" db_host = %s\n", db_host);
    printf(" db_user = %s\n", db_user);
    printf(" db_pass = %s\n", db_pass);
  }
  return (0);
} // end ParseConfig
