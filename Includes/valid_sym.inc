// valid_sym.inc
// verify the Symbol exists in the database else abort
#ifndef	_VALID_SYM
#define	_VALID_SYM 1
int valid_sym(char *Sym) {
  char query[1024];
  MYSQL_RES *result;
  MYSQL_ROW row=NULL;
  MYSQL_FIELD *field;
  unsigned long	*lengths;

  sprintf(query,"select distinct(symbol) from stockinfo where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) {
    printf("%s not found, not validated\n",Sym);
    return(EXIT_FAILURE);
  }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) {	// no results returned, 
    print_error(mysql, "Fatal error - valid_sym store results failed");
  } 
  if ((int)mysql_num_rows(result) == 0)  {
    printf("No entry found for symbol \"%s\" in stockinfo.\nPlease check the spelling, and run \"add_stock\" if it is a valid ticker symbol\n", Sym);
//    mysql_close(mysql);
    return(EXIT_FAILURE);
  }

  sprintf(query,"select distinct(symbol) from stockprices where symbol = \"%s\"",Sym);
  if (mysql_query(mysql,query)) {
    printf("%s not found, not validated\n",Sym);
    return(EXIT_FAILURE);
  }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) {	// no results returned, 
    print_error(mysql, "Fatal error - valid_sym store results failed");
  } 
  // fetch the row
  row=mysql_fetch_row(result);
  if (row == NULL) {
    printf("No data fetched for symbol  \"%s\" in stockprices.\nConsider running \"backpop\" if it is a valid ticker symbol\n", Sym);
    return(EXIT_FAILURE);
  }
  mysql_field_seek(result,0);
  field = mysql_fetch_field(result);
  lengths=mysql_fetch_lengths(result);
  if (!lengths[0] || row[0] == NULL) { printf("Invalid symbol %s or fetch of row failed",Sym); return(EXIT_FAILURE); }
  return(EXIT_SUCCESS);
}
#endif	/* !_VALID_SYM */
