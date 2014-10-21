// mysql error message printer
#ifndef _PRINT_ERROR
#define	_PRINT_ERROR 1
void print_error(MYSQL *mysql, char *message) {
  fprintf (stderr, "%s\n", message);
  if (mysql != NULL && mysql_errno(mysql)) fprintf (stderr, "Error %u: %s\n", mysql_errno(mysql), mysql_error(mysql));
  if (mysql != NULL) mysql_close(mysql);
  exit(EXIT_FAILURE);
}
#endif	/* !_PRINT_ERROR */
