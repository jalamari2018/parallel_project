//structure design is traken from the slides
typedef struct myDataStructure {
   int readers;                     
   int writer;                      
   pthread_cond_t canIRead;  // signaled when readers can proceed, somthing that can be read from the queue
   pthread_cond_t canIWrite;      // signaled when one of the writers can proceed, and write somthing to the queue
   int queueCount;                  // how many elements are in the queue
   int pending_readers;             // Readers waiting to read, we only have one writer so we do not need to use "pending_writer" as in the slide
   pthread_mutex_t readWriteLock; // locks myDataStructure
}myDataStructure;

void *producer(void *data);
void *consumer(void *data);

void enqueue(char ch);//put a character into the queue
void dequeue();//extract a character form the queue



typedef int bool;
