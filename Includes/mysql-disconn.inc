// disconnect from database and free memory
  
// finished with the database, close everything and free the memory
  if (mysql) {
    mysql_close(mysql);
    mysql_library_end();
  }
