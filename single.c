#include <stdio.h>
#include <stdlib.h>

#define N 128
 
int main()
{
  int c, sum = 0;
  int first[N], add[N];
 
 
  for (c = 0; c < N; c++)
  {
      first[c] = rand();  
  }
    
  for (c = 0; c < N; c++) {
      add[c] = first[c]+2;
  }
  return 0;
}
