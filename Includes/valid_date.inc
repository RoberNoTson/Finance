// valid_date.c
// get the closest valid date <= qDate for the Sym
#ifndef	_VALID_DATE
#define	_VALID_DATE 1
int valid_date(char * Sym) {
  char query[1024];
  MYSQL_FIELD *field_date;
  MYSQL_RES *result;
  MYSQL_ROW row;
  unsigned long	*lengths_date;

  sprintf(query, "select max(date) from stockprices where symbol = \"%s\" and date <= \"%s\"",Sym,qDate);
  if (mysql_query(mysql,query)) print_error(mysql, "Fatal error - valid_date failed to query database");
  // save the query results
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "Fatal error - valid_date store results failed");
  // fetch the row
  row=mysql_fetch_row(result);
  if(row==NULL) { print_error(mysql,"Fatal error - Invalid date or fetch of row failed"); }
  mysql_field_seek(result,0);
  field_date = mysql_fetch_field(result);
  lengths_date=mysql_fetch_lengths(result);
  if (!lengths_date[0] || row[0] == NULL)  print_error(mysql,"Fatal error - Invalid date or fetch of row failed, length=0");
  strcpy(qDate, row[0]);
  mysql_free_result(result);
  return(EXIT_SUCCESS);
}
#endif /* !_VALID_DATE */
