#include <pigpio.h>
void gpiostart(){
  FILE *fptr;

// Open a file in writing mode
fptr = fopen("log.txt", "w");

  if (gpioInitialise() < 0)
{
   fprintf(fptr, "Faileure Initilizing GPIO");
}
else
{
   fprintf(fptr, "GPIO initilized");
}
  fclose(fptr); 
}
