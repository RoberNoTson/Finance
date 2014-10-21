/* CNDL.c 
 * calculate numeric values for Candle chart entries
 * Two parameters are used to initialize the Bollinger Bands necessary to calculate all required thresholds;
 * the standard deviation number is set to 0.5 with a period of 55.
 * Conversion table: /Finance/gt/GT/Docs/CandelsticksCodes.txt
 * 
 * Parms: Sym [yyyy-mm-dd]
 * compile: gcc -Wall -O2 -ffast-math CNDL.c -o CNDL `mysql_config --include --libs`
*/

#define	DEBUG	0
#define	MAX_PERIODS	200
#define	Periods	55
#define	coeff	0.5

#define	_XOPEN_SOURCE
#include        <my_global.h>
#include        <my_sys.h>
#include        <mysql.h>
#include        <string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<errno.h>

#include        "Includes/print_error.inc"

int	Usage(char *prog) {
  printf("Usage: %s Sym [yyyy-mm-dd]\n",prog); 
  exit(EXIT_FAILURE);
}

int	main (int argc, char *argv[]) {
  int	cndl_code = 0;
  int	body_color = 0;
  int	num_rows;
  int	x,y;
  float	body=0,upper_shadow=0,lower_shadow=0;
  float	body_upper_thresh=0,body_lower_thresh=0,upper_shadow_upper_thresh=0,upper_shadow_lower_thresh=0,lower_shadow_upper_thresh=0,lower_shadow_lower_thresh=0;
  float	avg_body,avg_upper_shadow,avg_lower_shadow,sd_body,sd_UShad,sd_LShad;
  struct	stock {
    char	date[11];
    float	day_open;
    float	day_close;
    float	day_high;
    float	day_low;
  };
  struct	stock Stock[MAX_PERIODS+Periods];
  char	Sym[12];
  char	qDate[11];
  char	qDate2[11];
  char	query[1024];
  time_t t;
  struct tm *TM;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  MYSQL_FIELD *field;
  unsigned long	*lengths;
 
  // parse CLI parms
  if (argc == 1 || argc > 3) Usage(argv[0]);
  t = time(NULL);
  if ((TM = localtime(&t)) == NULL) { perror("localtime"); exit(EXIT_FAILURE); }
  if (!strftime(qDate, sizeof(qDate), "%F", TM)) { fprintf(stderr, "strftime returned NULL\n"); exit(EXIT_FAILURE); }
  if (argc >= 2) {
    memset(Sym,0,sizeof(Sym));
    for (x=0;x<strlen(argv[1]);x++) Sym[x] = toupper(argv[1][x]);
  }
  if (argc == 3) {
    if (sscanf(argv[2],"%1u%1u%1u%1u-%1u%1u-%1u%1u",&x,&x,&x,&x,&x,&x,&x,&x) == 8) {
      strcpy(qDate, argv[2]);
      if (!strptime(qDate,"%F",TM)) { fprintf(stderr, "Invalid date %s\n",argv[2]); exit(EXIT_FAILURE); }
      if (!strftime(qDate2, sizeof(qDate2), "%F", TM)) { fprintf(stderr, "strftime returned NULL\n"); exit(EXIT_FAILURE); }
      if (strcmp(qDate,qDate2)) { fprintf(stderr, "Invalid date %s\n",argv[2]); exit(EXIT_FAILURE); }
    } else Usage(argv[0]);
  }  
  
  // get the data
  #include "Includes/beancounter-conn.inc"
  sprintf(query,"select date,day_open,day_high,day_low,day_close from stockprices \
    where symbol = \"%s\" and date <= \"%s\" order by date desc limit %d",
	  Sym,qDate,MAX_PERIODS+Periods-1);
  if (DEBUG) printf("%s\n",query);
  if (mysql_query(mysql,query)) print_error(mysql, "Failed to query database");
  result=mysql_store_result(mysql);
  if ((result==NULL) && (mysql_errno(mysql))) print_error(mysql, "store_results failed"); 
  num_rows=mysql_num_rows(result);
  if (num_rows == 0)  {
    printf("0 rows found for symbol %s\n",Sym);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }
  if (num_rows < (MAX_PERIODS+Periods)-1) {
    printf("Not enough rows found (%d) to process Standard Deviation for %d periods\n",num_rows,Periods);
    mysql_close(mysql);
    exit(EXIT_FAILURE);
  }

  // loop to load structs
  for (x = num_rows; x>0; x--) {
    if ((row=mysql_fetch_row(result)) == NULL) print_error(mysql,"00 Fetch error, exiting");
    // error check for nulls
    if(row==NULL) { mysql_free_result(result); print_error(mysql,"01 Oops!  null data found and skipped\n"); }
    mysql_field_seek(result,0);
    field = mysql_fetch_field(result);
    lengths=mysql_fetch_lengths(result);
    if (!lengths[0]) { mysql_free_result(result);print_error(mysql,"02 Oops!  null data found and skipped\n");  }
    if (row[0] == NULL) { mysql_free_result(result); print_error(mysql,"03 Oops!  null data found and skipped\n");  }
    if (!lengths[1]) { mysql_free_result(result); print_error(mysql,"02 Oops!  null data found and skipped\n"); ; }
    if (row[1] == NULL) { mysql_free_result(result); print_error(mysql,"03 Oops!  null data found and skipped\n");  }
    if (!lengths[2]) { mysql_free_result(result); print_error(mysql,"02 Oops!  null data found and skipped\n");  }
    if (row[2] == NULL) { mysql_free_result(result); print_error(mysql,"03 Oops!  null data found and skipped\n");  }
    if (!lengths[3]) { mysql_free_result(result); print_error(mysql,"02 Oops!  null data found and skipped\n");  }
    if (row[3] == NULL) { mysql_free_result(result); print_error(mysql,"03 Oops!  null data found and skipped\n");  }
    if (!lengths[4]) { mysql_free_result(result); print_error(mysql,"02 Oops!  null data found and skipped\n");  }
    if (row[4] == NULL) { mysql_free_result(result); print_error(mysql,"03 Oops!  null data found and skipped\n"); }
    strcpy(Stock[x].date,row[0]);
    Stock[x].day_open = strtof(row[1],NULL);
    Stock[x].day_high = strtof(row[2],NULL);
    Stock[x].day_low = strtof(row[3],NULL);
    Stock[x].day_close = strtof(row[4],NULL);
  }	// end For
  // finished with the database
  mysql_free_result(result);
  #include "Includes/mysql-disconn.inc"

  // calculate the sizes and thresholds
  for (x=0; x <= (num_rows-Periods); x++) {
    cndl_code = 0;
    body_color = 0;
    avg_body = 0;
    avg_upper_shadow = 0;
    avg_lower_shadow = 0;
    for (y=x; y<(x+Periods); y++) {
    // Calculate the size of : Body, Lower and Upper Shadows  
      avg_body += fabsf(Stock[y].day_open - Stock[y].day_close);
      avg_upper_shadow += (Stock[y].day_close >= Stock[y].day_open) ? 
	(Stock[y].day_high - Stock[y].day_close) : (Stock[y].day_high - Stock[y].day_open);
      avg_lower_shadow += (Stock[y].day_close >= Stock[y].day_open) ? 
	(Stock[y].day_open - Stock[y].day_low) : (Stock[y].day_close - Stock[y].day_low);
    }
    avg_body /= Periods;
    avg_upper_shadow /= Periods;
    avg_lower_shadow /= Periods;

    // Get all thresholds
    sd_body=0.0;
    sd_UShad=0;
    sd_LShad=0;
    for (y=x; y<(x+Periods); y++) {
      sd_body += pow((fabsf(Stock[y].day_open - Stock[y].day_close)-avg_body),2);	// body
      sd_UShad += pow((((Stock[y].day_close >= Stock[y].day_open) ? (Stock[y].day_high - Stock[y].day_close) : (Stock[y].day_high - Stock[y].day_open))-avg_upper_shadow) ,2);
      sd_LShad += pow((((Stock[y].day_close >= Stock[y].day_open) ? (Stock[y].day_open - Stock[y].day_low) : (Stock[y].day_close - Stock[y].day_low))-avg_lower_shadow) ,2);
    }
    sd_body = sqrtf(sd_body/Periods);
    sd_UShad = sqrtf(sd_UShad/Periods);
    sd_LShad = sqrtf(sd_LShad/Periods);
    body = fabsf(Stock[y].day_open - Stock[y].day_close);
    upper_shadow = (Stock[y].day_close >= Stock[y].day_open) ? (Stock[y].day_high - Stock[y].day_close) : (Stock[y].day_high - Stock[y].day_open);
    lower_shadow = (Stock[y].day_close >= Stock[y].day_open) ? (Stock[y].day_open - Stock[y].day_low) : (Stock[y].day_close - Stock[y].day_low);
    body_upper_thresh = avg_body + (coeff * sd_body);
    body_lower_thresh = avg_body - (coeff * sd_body);
    upper_shadow_upper_thresh = avg_upper_shadow + (coeff * sd_UShad);
    upper_shadow_lower_thresh = avg_upper_shadow - (coeff * sd_UShad);
    lower_shadow_upper_thresh = avg_lower_shadow + (coeff * sd_LShad);
    lower_shadow_lower_thresh = avg_lower_shadow - (coeff * sd_LShad);

    // Determine the candelstick code with a sequence of seven binary digits
    // Step 1 : Candelstick Color
    // The first binary digit represents the color of the candelstick : one (1) for a white one, and zero (0) for a black one.
    // A Doji candelstick - which as zero body size - is white if its upper shadow is longer than its lower shadow.
    if (((Stock[y].day_close > Stock[y].day_open)) || 
      ((body==0)  && (upper_shadow > lower_shadow))) {
	body_color = 1;
	cndl_code += 64; //pow(2,6)
    }
  
    // Step 2 : Candelstick Body
    // The second position, composed of two binary digits, denotes body size, depending on the first digit, or color, of the body.
    // Candelsticks	    White   Black
    // No body size (doji)   00	    11
    // Small body	    01	    10
    // Middle body	    10	    01
    // Large body	    11	    00
    if (((body == 0) && (upper_shadow <= lower_shadow))
	||
	((body_color == 1) && (body >= body_upper_thresh)))
    {
	cndl_code += 48; // pow(2,5) + pow(2,4)
    }
    if (((body_color == 0) && (body_lower_thresh <= body) &&
	 (body < body_upper_thresh)) 
	|| 
	((body_color == 1 ) && (body < body_lower_thresh) &&
	 (body > 0)))
    {
	cndl_code += 16; // pow(2,4)
    }
    if (((body_color == 0) && (body < body_lower_thresh) &&
	 (body > 0)) 
	|| 
	((body_color == 1) && (body_lower_thresh <= body) && 
	 (body < body_upper_thresh)))
    {
	cndl_code += 32; // pow(2,5)
    }
    
    // Step 3 : Candelstick Shadows
    // The fird and the fourth positions (each consisting of two binay digits) code the sizes of upper and lower shadows, repecitvely :
    //
    // Shadows	Upper   Lower
    // None	00	11
    // Small	01	10
    // Middle	10	01
    // Large	11	00
    if ((upper_shadow < upper_shadow_lower_thresh) &&
        (upper_shadow > 0)) {
	cndl_code += 4; // pow(2,2)
    }
    if ((upper_shadow_lower_thresh <= upper_shadow) &&
	(upper_shadow < upper_shadow_upper_thresh)) {
	cndl_code += 8; // pow(2,3)
    }
    if (upper_shadow >= upper_shadow_upper_thresh) {
	cndl_code += 12; // pow(2,3) + pow(2,2)
    }
    if ((lower_shadow < lower_shadow_lower_thresh) &&
	(lower_shadow > 0)) {
	cndl_code += 2; // pow(2,1)
    }
    if ((lower_shadow_lower_thresh <= lower_shadow) &&
	(lower_shadow < lower_shadow_upper_thresh)) {
	cndl_code += 1; // pow(2,0)
    }
    if (lower_shadow == 0) {
	cndl_code += 3; // pow(2,1)+ pow(2,0)
    }
    // Return the results
    printf("[%s] =\t%d\n",Stock[y].date,cndl_code);
  }	// end For
  
  exit(EXIT_SUCCESS);
}
