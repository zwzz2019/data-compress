#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#define SIZE 100

int main(){
  int i,j,u,v,n,l;
  float Qstep[52] = {0.625,0.6875,0.8125,0.875,1,1.125,1.25,1.375,1.625,1.75,2,2.25,2.5,2.75,3.25,3.5,4,4.5,5,5.5,6.5,7,8,9,10,11,13,14,16,18,20,22,26,28,32,36,40,4,52,56,64,72,80,88,104,112,128,144,160,176,208,224};
  float dct[SIZE],in[SIZE],idct[SIZE];
  float step;
  printf("Level of quantization\t");
  scanf("%d",&l);
  printf("Step is %.4f\t", Qstep[l]);
  step = Qstep[l];
  printf("Size of DCT\t");
  scanf("%d",&n);
  if( n < SIZE )
    printf("Enter the numbers\n");
  else{
    printf("Out of Sizes\n");
    exit(1);
  }

  for(i = 0;i < n;i++)
    scanf(" %f",&in[i]);
  float csum = 0;
  printf("\nFDCT is ");
  for(i = 0;i < n;i++){
    for(j = 0;j < n;j++)
      csum += in[j]*cos((2*j+1)*M_PI*i/(2*n));
    dct[i] = i == 0 ? (csum/sqrt(2)) : csum;
    csum = 0;
    if (i % 8 == 0) {
      printf("\n");
    };
    printf("%0.3f ",dct[i]);
}

  printf("\n\nQuantization  is:");
  for(i = 0;i < n;i++){
    if (i % 8 == 0) {
      printf("\n");
    };
    dct[i] = round(dct[i]/step);
    printf("%.0f ",dct[i]);
  }

  printf("\n\ni-Quantization  is:");
  for(i = 0;i < n;i++){
    if (i % 8 == 0) {
      printf("\n");
    };
    dct[i] = dct[i]*step;
    printf("%.3f ",dct[i]);
  }

  printf("\n\nIDCT is:");
  float k = 0;
  for(i = 0;i < n;i++){
    for(j = 0;j < n;j++){
      k = dct[j]*cos((2*i+1)*M_PI*j/(2*n));
      csum += (j == 0) ? (k/sqrt(2)) : k;
    }
    idct[i] = csum*2/n;
    csum = 0;
    if (i % 8 == 0) {
      printf("\n");
    };
    printf("%0.3f ",idct[i]);
  }
  printf("\n");

  return 0;
}

