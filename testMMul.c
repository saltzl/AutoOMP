#include <stdio.h>
#include <stdlib.h>

#define N 128
 
int main()
{
  int c, d, k, sum = 0;
  int first[N][N], second[N][N], multiply[N][N];
 
 
  for (c = 0; c < N; c++)
  {
    for (d = 0; d < N; d++)
    {
      first[c][d] = rand()/ RAND_MAX;  
      second[c][d] = rand()/ RAND_MAX;  
    }
  }
    
  for (c = 0; c < N; c++) {
      for (d = 0; d < N; d++) {
        for (k = 0; k < N; k++) {
          sum = sum + first[c][k]*second[k][d];
        }
 
        multiply[c][d] = sum;
        sum = 0;
      }
    }
  return 0;
}
