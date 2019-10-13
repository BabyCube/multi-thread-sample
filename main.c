
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>

/*************************************************************************************************************************************/
struct IDData{
    
    int thread_id; /* stores the ID number */
   
};

int currentID = 1; /* the current ID */

struct IDData thread_data_array[4];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int printNcount(void *i); /* pre-define the function for printNcount here*/

int CurrentID(){
    
    pthread_t thread[4]; /* four threads generated, corresponding to four IDs*/
    
    /* the argument to be passed on to threads are stored into the struct */
    for(int i = 0; i < 4; i++){
        
        thread_data_array[i].thread_id = i + 1;
        pthread_create(&thread[i], NULL, printNcount, (void *)&thread_data_array[i]);
    }
    /* the last thread to finish would be the 4th thread; wait for it to finish before continuing */
    pthread_join(thread[3], NULL);
    return 0;
    
}

int printNcount(void *i){
    
    int count = 0;
    int check = 1;
    int IDNumber = 0;
    
    struct IDData *myData; /* extracting the data from the struct */
    myData = (struct IDData *)i;
    IDNumber = myData->thread_id;
    
    while(count < 10){
        
        
        pthread_mutex_lock(&mutex);
        printf("My ID is %d . \n ", IDNumber);
        
        if(IDNumber == currentID){
            
            printf("My turn! Turns left before stop: %d \n \n", (9 - count));
            count++;
            currentID++; /* increment CurrentID by one if it is its turn! */
            
            if(currentID == 5){ /* if 5, then reset the value before release the variable */
                
                currentID = 1; /* notice all these process should be done when locked to prevent overwrite */
            }
            
            pthread_mutex_unlock(&mutex);
        }else{
            
            printf("Not my turn. My ID %d but the currentID %d. Turns left before stop: %d \n",IDNumber, currentID,(10 - count));
            printf("This is the %i th time checking failed! \n \n",  check); /* keep track of how many times the ID has been compared*/
            check++;
            pthread_mutex_unlock(&mutex);
            
            if(count == 10){
                return 0;
            }
        }
        
    }

    return 0;
}

/****************************************This is the end for the first part of the assignment*****************************************/

int sizeProduction = 0;/* size of the minimum production and consumption */
int queueSize = 0; /* size of the buffer */
int producerSleepCounter = 0; /* these two variables count the times that the producer and the consumer goes to sleep */
int consumerSleepCounter = 0;
int produced = 0; /* these two variables will keep track of items produced and consumed; both have to be larger than sizeProduction when program ends */
int consumed = 0;
int currentItems = 0;
int producers = 0;
int consumers = 0;

sem_t *full;
sem_t *empty;
sem_t *producerMutex; /* one more semaphore has to be created to ensure producers synchronize within themselves */
sem_t *consumerMutex; /* one more semaphore has to be created to ensure consumers synchronize within themselves */

struct producerID{
    
    int producerID; /* keep track of which consumer it is */
    sem_t *fullPointer;
    sem_t *emptyPointer;
    
};

struct consumerID{
    
    int consumerID; /* keep track if which producer it is */
    sem_t *fullPointer;
    sem_t *emptyPointer;
    
};

int producer(void *i);
int consumer(void *i);


void producer_consumer(){
    
    printf("Please enter the queue size: (must be a positive integer) \n");
    scanf("%d", &queueSize); /* the user will enter the size of the queue here */
    printf("Please enter the least number to be produced and consumed: \n");
    scanf("%d", &sizeProduction);
    printf("Please enter the number of producers in the model: \n");
    scanf("%d", &producers);
    printf("Please enter the number of consumers in the model: \n");
    scanf("%d", &consumers);
    
    /* two semaphores should be used here. full, empty */
    /* initilizating the semaphores */
    /* compiled and runned in MAC; unnamed semaphores are not supported; sem_init() would not work */
    /* if the semaphore does not exist, it will be created; user has the access to execute, read and modify */
    sem_unlink("/full");
    sem_unlink("/empty");
    sem_t * full = sem_open("/full", O_CREAT|O_EXCL, S_IRWXU, 0);
    sem_t * empty = sem_open("/empty", O_CREAT|O_EXCL, S_IRWXU, queueSize);
    
    /* consumer threads are created here; threads number depending on number of producers */
    struct consumerID consumer_data_array[consumers];
    struct producerID producer_data_array[producers];
    pthread_t consumerThread[consumers];
    pthread_t producerThread[producers];
    
    /* producer threads are created here; threads number depending on number of producers */
    for(int i = 0; i < producers; i++){
        producer_data_array[i].producerID = i + 1;
        producer_data_array[i].emptyPointer = empty;
        producer_data_array[i].fullPointer = full;
        pthread_create(&producerThread[i], NULL, producer, (void *)&producer_data_array[i]);
    }
    
    for(int i = 0; i < consumers; i++){
        consumer_data_array[i].consumerID = i + 1;
        consumer_data_array[i].emptyPointer = empty;
        consumer_data_array[i].fullPointer = full;
        pthread_create(&consumerThread[i], NULL, consumer, (void *)&consumer_data_array[i]);
    }
    
    
    pthread_join(consumerThread[consumers - 1], NULL); /* wait for all the threads to finish before continue */
    pthread_join(producerThread[producers - 1], NULL);
    sem_close(full);
    sem_close(empty);
    printf("\n \nConsumers have been placed into sleep for %d times. \n", consumerSleepCounter);
    printf("Producers have been placed into sleep for %d times. \n", producerSleepCounter);
    printf("%d of items has been consumed and %d of items has been produced in total. \n", consumed, produced);
    
}

int producer(void *i){
    int IDNumber = 0;
    sem_t *myfull = NULL;
    sem_t *myempty = NULL;
    
    struct producerID *myData; /* extracting the data from the struct */
    myData = (struct producerID *)i;
    IDNumber = myData->producerID;
    myfull = myData->fullPointer;
    myempty = myData->emptyPointer;
    
    
    while(1){
        
        if(currentItems == 0){
            consumerSleepCounter++; /* manually count the times that consumers have been put to sleep */
        }
        
        if(consumed > sizeProduction && produced > sizeProduction){
            return 0;
        }
        
        int k = sem_wait(myempty); /* producer blocked if full (empty = 0), or decrement empty by 1 */
        printf("Producer Wait for empty slot semaphore %d \n", k); /* if 0, semaphore successful; if 1, semaphore failed */
        pthread_mutex_lock(&mutex); /* enter critical region */
        printf("this is producer's turn! count before produced %d My producerID %d \n", currentItems, IDNumber);
        produced++;
        currentItems++;
        pthread_mutex_unlock(&mutex); /* leave critical region */
        sem_post(myfull); /* wakes the consumer up if empty (full = 0), or increment full by 1*/
        
    }
    return 0;
}

int consumer(void *i){
    int IDNumber = 0;
    sem_t *myfull = NULL;
    sem_t *myempty = NULL;
    
    struct consumerID *myData; /* extracting the data from the struct */
    myData = (struct consumerID *)i;
    IDNumber = myData->consumerID;
    myfull = myData->fullPointer;
    myempty = myData->emptyPointer;
    
    while(1){
        
        if(currentItems == queueSize){
            producerSleepCounter++; /* manually count the times that producers have been put to sleep */
        }
        
        if(consumed > sizeProduction && produced > sizeProduction){
            return 0;
        }
        
      int k =  sem_wait(myfull); /* consumer blocked if empty (full = 0), or decrement full by 1 */
        printf("Consumer wait for full slot semaphore %d \n", k); /* if 0, semaphore successful; if 1, semaphore failed */
        pthread_mutex_lock(&mutex); /* enter critical region */
        printf("this is consumer's turn! count before consumed %d My consumerID %d \n", currentItems, IDNumber);
        consumed++;
        currentItems--;
        pthread_mutex_unlock(&mutex); /* leave critical region */
        sem_post(myempty); /* wakes up the producer if empty (empty = 0), or increment empty by 1 */
        
    }
    return 0;
}

/**************************************This is the end for the second part of the assignment*****************************************/

int main(int argc, const char * argv[]) {
    
    printf("Please enter which part to run: 1 - CurrentID problem; 2 - Producer-consumer problem \n");
    printf("Enter 1 or 2 (enter 0 to quit)\n");
    
    int selector = 5;
    scanf("%d", &selector);
    
    if(selector == 1){
        CurrentID();
    }else if(selector == 2){
        producer_consumer();
    }
    
    return 0;
}
