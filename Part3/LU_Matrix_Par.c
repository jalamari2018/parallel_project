#include <omp.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//functions
void procedureLU(double **A, int size);
void intializeArray(double ***A, int size);
void printArray(double **A, int size);
int readFile(double ***A);


int main(int argc, char *argv[]){
  double **A;  
  int size=3;
  int i, j;

  size = readFile(&A);
  procedureLU(A, size);
  return 0;
}

void procedureLU(double **A, int size){
  int i, j, k;

//printf("Started\n");
  #pragma omp parallel private(k, i, j)
  {
    
    for(k=0;k<size;k++){
       #pragma omp for
        for(j=(k+1);j<size;j++){
          A[k][j] = A[k][j] / A[k][k];
        }
        #pragma omp for
        for(i=(k+1);i<size;i++){
          for(j=(k+1);j<size;j++){
            A[i][j] = A[i][j] - (A[i][k] * A[k][j]);
          }  
        }
    }
  }

 printArray(A,size);

}


/// functions implementation ///

void intializeArray(double ***A, int size){
  int i,j;
  //Allocate 2-D matrix
  *A = (double **) malloc(size * sizeof(double *));
 // printf("init started\n");
  for(i=0;i<size;i++){
    (*A)[i] = (double *) malloc(size * sizeof(double));
  }
//fill with zeros
  for(i=0;i<size;i++){
    for(j=0;j<size;j++){
      (*A)[i][j] = 0.0;
    }
  }
//    printf("init ended\n");
}

int readFile(double ***A){
 // printf("READ FILE started\n");
  double **atmp;// tempo
  FILE *fr;
  double tmp;
  char *line, *tok;
  int size, i, j,lineSize = 512;
  line = malloc(lineSize * sizeof(char));
  fr = fopen("data.txt", "rt");
  if(fgets(line, lineSize, fr) != NULL) {
    size = atoi(line);
   //  printf("size = %d \n", size);
    intializeArray(A,size);
  }
  fgets(line, lineSize, fr); // to get the empty line between the array and the size.
  j=0;
  // Read through the file and get Matrix elements
  atmp = (double **) malloc(size * sizeof(double *));
 
  for(i=0;i<size;i++){
  //  printf("TEST \n");
    atmp[i] = (double *) malloc(size * sizeof(double));
    if(fgets(line, 120, fr) != NULL) {
      tok = (char *) strtok(line, " ");
      while(tok) {
       // printf("Test INNER\n");
        tmp = atoi(tok);
        atmp[i][j] = (double)tmp;
        tok = (char *) strtok(NULL, " ");
        j++;
      }
      
      j=0;
    }
  }
  fclose(fr);
  *A = atmp;//
  //printf("READ FILE ENded\n");
  return size;
}


void printArray(double **A, int size){
  int i,j;
  for(i=0;i<size;i++){
    for(j=0;j<size;j++){
      printf("%lf \t", A[i][j]);
    }
    printf("\n\n");
  }
}



