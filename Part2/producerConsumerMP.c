//Producer consumer using OpenMP
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

char workBuffer;
int available = 0;
int done = 0;

int main(int argc, char* argv[]) {
 

  #pragma omp parallel sections shared (workBuffer) shared (available) shared (done)
  {
  
    #pragma omp section //producer section
    {
      
      int i=0,stringLength, fileSize = 1024;
      FILE *fr = NULL;
      char character, *line = malloc(512*sizeof(char));
      fr = fopen("string.txt", "rt");
      while(fgets(line, fileSize, fr) != NULL) {
        stringLength = strlen(line);
        for(i=0;i<stringLength;i++){
          int inserted = 0;
          #pragma omp critical(queue)
          {
            if(!available)// if the workBuffer has no item, could at the start of the program or after the item was consummed by the consumer
            {
              if(line[i] != '\n' && line[i] !=' '){
                  workBuffer = line[i];//insert character to the workBuffer
                  available = 1;
                  printf("[%c]Produced +\n",workBuffer);
               }
              inserted = 1;
            }
          }
          if(!inserted){// check if insertion was successful, if not, it decreases the index, so the character will be inserted in the following iteration
            i--;
          }
        } //inner loop for characters in a single line.
      } // outer loop for lines in the file.
      fclose(fr);

      #pragma omp critical(isDone)
      {
        done = 1;
      }


    } //end

    #pragma omp section //consumer section
    {
      int isFinished = 0;
      char character;
      while(!isFinished) {
        #pragma omp critical(queue)
        {
          if(available)
          {
            character = workBuffer;
            printf("[%c]Consumed -\n  -------- \n",character);
            available = 0;
          }
        }

        #pragma omp critical(isDone)
        {
          isFinished = done;
        }
      } //while

    } 

    
  } 

}
