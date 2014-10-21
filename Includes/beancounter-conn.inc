// Connect to the MySQL database
// #include this in the program

  char  cnf_file[200];
  mysql_library_init(-1,NULL,NULL);
  mysql = mysql_init(NULL);
  if (mysql == NULL) {
    print_error(NULL, "mysql_init() failed: check for out-of-memory condition");
    return(1);
  }
  strcpy(cnf_file, getenv("HOME"));
  strcat(cnf_file, "/mysql.cnf");
  mysql_options(mysql,MYSQL_READ_DEFAULT_FILE,cnf_file);
#ifdef RELEASE
  if (RELEASE) mysql_options(mysql,MYSQL_READ_DEFAULT_GROUP,"prod");
  if (RELEASE==0) mysql_options(mysql,MYSQL_READ_DEFAULT_GROUP,"test");
#else
  mysql_options(mysql,MYSQL_READ_DEFAULT_GROUP,"prod");
#endif
  if (!mysql_real_connect(mysql,NULL,NULL,NULL,NULL,0,NULL,0)) {
    print_error(mysql, "Failed to connect to database");
    mysql_close(mysql);
    return(1);
  }
  
