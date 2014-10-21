// holiday_check.inc
// check if requested date is a business holiday or weekend, exit/abort processing if true
// Parm is either a date string (yyyy-mm-dd) or NULL to use today

int holiday_check(char *qDate) {
  char mDate[12];
  unsigned long	*lengths;
  time_t t;
  struct tm *TM;

  t = time(NULL);
  TM = localtime(&t);
  if (TM == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }
  if (qDate == NULL) {
    if (strftime(mDate, sizeof(mDate), "%F", TM) == 0) {
      fprintf(stderr, "in holiday_check strftime returned 0");
      exit(EXIT_FAILURE);
    }
  } else {
    strptime(qDate,"%F", TM);
    strcpy(mDate,qDate);
  }
  // skip weekends
  if (TM->tm_wday < 1 || TM->tm_wday > 5) {
    fprintf(stderr,"Date %s is a weekend, aborting processing\n",mDate);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

  // check for and skip business holidays
  if (mysql_query(mysql,"select distinct(bus_holiday) from Investments.holidays where bus_holiday <= CURRENT_DATE order by bus_holiday")) {
    print_error(mysql,"Fatal Error - parsing holidays");
  }
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "Fatal error - check_holiday store results failed");
  if ((int)mysql_num_rows(result) == 0)  print_error(mysql, "Fatal error - no rows found for holiday_check");
  while ((row=mysql_fetch_row(result))) {
    if (row == NULL) print_error(mysql, "Fatal error - no rows found for holiday_check");
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"Invalid date or fetch of row failed"); }
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result); print_error(mysql,"Invalid date or fetch of row failed"); }
    if ((strcmp(row[0],qDate))==0) {
      printf("Date %s is a holiday, abort processing\n",mDate);
      mysql_close(mysql);
      exit(EXIT_FAILURE);
    }
  }
  mysql_free_result(result);
  return(EXIT_SUCCESS);
}
