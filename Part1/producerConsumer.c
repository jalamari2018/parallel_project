#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "producerConsumer.h" // has function declarations, and the data structure.
#define SIZE 5
#define true 1
#define false 0

char *workingQueue; 
int  readerIndex =0, writerIndex =0,done=0,incerement=0; 
 
struct myDataStructure myStructure;
//these locks are in the global space so they are available to all threads
pthread_mutex_t doneLock; //used to protect done global variable
pthread_mutex_t readerIndexLock;//used to protect readerIndex variable because it is going to changed by readers
 
int main (int argc, char* argv[])
{
	int TH, numberOfreaders,i;
	//allocate working queue 
	workingQueue = malloc(SIZE * sizeof(char));
	//before creating threads we need to intialize locks
	pthread_mutex_init(&doneLock, NULL);
	pthread_mutex_init(&readerIndexLock,NULL);

	//initialize the data structure
	 myStructure.readers = myStructure.writer= myStructure.queueCount = 0;//set to zeros
	 pthread_mutex_init(&(myStructure.readWriteLock), NULL);
	 pthread_cond_init(&(myStructure.canIWrite), NULL);
	 pthread_cond_init(&(myStructure.canIRead), NULL);
	

	 if(argc != 2){
	 	printf("Please Enter the number of threads as the first ARGUMENT\n");
	 	return(-1);
	 }else if(atoi(argv[1]) < 2){
	 	printf("Number of threads should be [2] at least\n");
	 }else{
	 	numberOfreaders = atoi(argv[1]);
	 }
	 pthread_t *threadsArray = malloc( (numberOfreaders) * sizeof(pthread_t) );
	 TH = pthread_create(&threadsArray[0], NULL, producer, NULL);
	 if(TH){
	 	printf("Faild\n");
	 }
	 //create readers
	 for(i=1;i<numberOfreaders;i++){
	 	TH = pthread_create(&threadsArray[i], NULL, consumer, (void *)&i);
	 	if(TH){
	 		printf("Faild\n");
	 	}
	 }
	 //wait for threads to join
	 for(i=0;i<numberOfreaders;i++){
	 	 TH = pthread_join(threadsArray[i], NULL);
	 }

return 0;
}



// functions implementation

void *producer(void *data){
	// printf("Producer \n");

	//read the file
	int i,stringLength, stringSize = 512;// because the text file is not very big
	FILE *fr;
	//allocate a location for the line in the memory 
	char *line = malloc(stringSize* sizeof(char));
	//open the file 
	fr = fopen("string.txt", "rt");
	//get the line from the file
	while(fgets(line, stringSize, fr) != NULL){
		// we got the line
	
		stringLength = strlen(line);// get the length of the line
		for(i=0;i<stringLength-1;i++){ // -1 here is to avoid reading the end of line '\n'

				/* we enter the character to the queue, but before we do that we need to lock the queue, using the lock in the data structure,
				 at the same tim the lock is used to prtect members of the data structure
				*/

				//first get the lock
			pthread_mutex_lock(&(myStructure.readWriteLock));
			//printf("producer got the lock\n");
			// if suceeded we and there is a place to write in 
			while(myStructure.queueCount >= SIZE){
				//that means I cannot write to the buffer, so I should stop here and wait
				//pthread_cond_broadcast(&(myStructure.canIRead));// brodcast because the queue is full
				pthread_cond_wait(&(myStructure.canIWrite),&(myStructure.readWriteLock));
				// pthread_mutex_lock(&(myStructure.readWriteLock));
			}
		    enqueue(line[i]);//call enqueue to add the character to the queue
		    myStructure.queueCount++;
		    pthread_cond_broadcast(&(myStructure.canIRead));// brodcast because the queue is full
			pthread_mutex_unlock(&(myStructure.readWriteLock));
			//printf("Producer releases the lock\n");
				//printf("Charcter I read is '%c'\n", line[i]);
		}//reading characters from the line to be inserted in the queue, TODO stingLength could be used later
		//pthread_cond_broadcast(&(myStructure.canIRead));
	}//Reading Lines


    
   pthread_mutex_lock(&doneLock);
    	//printf("Producer got doneLock\n");
        done = 1;//tells readers that I am done so they do not wait for my signals or signal me
    pthread_mutex_unlock(&doneLock);
//printf("Producer Releases doneLock\n");
    printf("Producer is done, going home\n");
    pthread_mutex_lock(&(myStructure.readWriteLock));
       pthread_cond_broadcast(&(myStructure.canIRead));// brodcast because the queue is full
    pthread_mutex_unlock(&(myStructure.readWriteLock));
	pthread_exit(NULL);
}

void *consumer(void *data){
	//printf(" consumer [%ld] started\n",pthread_self());
	bool iamDone = false;
	int producerDone = 0;
	while(iamDone == false){
		pthread_mutex_lock(&doneLock);
		  producerDone = done;//test tp see if producer is sleeping on the condition
		//printf("consumer got doneLock\n");
		pthread_mutex_unlock(&doneLock);
		//try to obtain the lock
		pthread_mutex_lock(&(myStructure.readWriteLock));
		//printf("Consumer got the lock\n");
		while(myStructure.queueCount == 0 && producerDone == false){
			//that means there is nothing to read
				//printf("Consumer slept id [%ld]\n", pthread_self());
				myStructure.readers++;
				pthread_cond_signal(&(myStructure.canIWrite));//signal producer to work
				pthread_mutex_lock(&doneLock);
				  producerDone = done;//test tp see if producer is sleeping on the condition
				//printf("consumer got doneLock\n");
				pthread_mutex_unlock(&doneLock);
				if(producerDone == false){
					pthread_cond_wait(&(myStructure.canIRead),&(myStructure.readWriteLock));//sleep on the condition canIRead
				 //   printf("Nothing left to do lets go home\n");
				}


		}
		//printf("Hello there \n");
		if(myStructure.queueCount != 0){// to prevent trying to dequeue form empty buffer when done, in the last itteration of while
			dequeue(); // to dequeue the character from the buffer, do not worry the lock is there
			myStructure.readers--;
			myStructure.queueCount--;
	    }
	    if(producerDone == true && myStructure.queueCount == 0){
	    	iamDone = true;
	    }
	  pthread_mutex_unlock(&(myStructure.readWriteLock));//release the lock
	  //printf("Consumer releases the lock\n");

	}
	printf("Consumer %ld,  going home\n",pthread_self());
	pthread_exit(NULL);
}







////////////////////////////////////////////////////////////////////////////////////////// queue functions ///////////////////////////////////////////////
void enqueue(char character){
	workingQueue[writerIndex] = character;
	//printf("I wrote %c to %d\n", character ,writerIndex);
	writerIndex++;
	writerIndex = writerIndex % SIZE; // so it goes in circle
	//printf("My next index that I am going to write to is %d\n", writerIndex);
}


void dequeue(){
	char character;
	// we need to lock readerIndex, to organize reading
	pthread_mutex_lock(&readerIndexLock);
	//printf("from dequeue\n");
		character = workingQueue[readerIndex];
		readerIndex++;
		readerIndex = readerIndex % SIZE; // move the reader
	pthread_mutex_unlock(&readerIndexLock);
	printf("I am thread [%ld]  and I consumed '%c'\n", pthread_self(),character);
}

