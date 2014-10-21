/* median.c
 * Calculate the median value of args
 * Parms: ##.## [...]
 * 
 * If even number of parms, calculate the average value,
 * if odd number find the middle value.
 * compile:  gcc -Wall -O2 -ffast-math median.c -o median
 */

#define		DEBUG 1

#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>

float * array;

static int value_sort(float *i, float *j)
{
   if (*i - *j > 0)
         return(1);
   if (*i - *j < 0)
      return(-1);
   return(0);
} // end value_sort()

int main(int argc, char *argv[]) {
  int	x;
  float sum=0;

  if (argc == 1) {
    printf("Usage: %s #.# [#.# ...]\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  // even number of parms
  if ((argc % 2) == 1) {
    sum = 0;
    for (x=1;x<argc;x++) sum += strtof(argv[x],NULL);
    sum /= (argc-1);
  }
  // odd number of parms, find middle value
  else {
    array = calloc((argc-1),sizeof(float));
    for (x=0;x<(argc-1);x++) array[x] = strtof(argv[x+1],NULL);
    qsort(array, argc-1, sizeof(float), (void *)value_sort);
    sum = array[(argc/2)-1];
    free(array);
  }
  printf("%f\n",sum);
  exit(EXIT_SUCCESS);
}
