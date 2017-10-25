//Shambhavi Srivastava

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "util.h"
#include "queue.h"
#include "multi-lookup.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

void* requestThread(void* id){
struct thread* thread = id; //Make a thread to hold info
   	char hostname[MAX_NAME_LENGTHS]; //hostname char arrays 
	char *domain;
	bool done = false; //flag
	FILE* inputfp = thread->threadFile; //Input file
	pthread_mutex_t* buffMutex = thread->buffMutex; //Buffer mutex
	queue* hostQueue = thread->hostQueue; //Queue

   	while(fscanf(inputfp, INPUTFS, hostname) > 0){ //Read input and push int buffer
		while(!done){ //Repeat until hostname is pushed to the queue
			domain = malloc(SBUFSIZE); //allocate memory for domain (or hostname)
			strncpy(domain, hostname, SBUFSIZE); //copy hostname to domain, max number of characters
			pthread_mutex_lock(buffMutex);//Lock/block the buffer
			while(queue_is_full(hostQueue)){ //When queue is full
					pthread_mutex_unlock(buffMutex); //Unlock/block buffe
					usleep(rand() % 100 + 5); //Wait 0-100 microseconds
					pthread_mutex_lock(buffMutex); //Lock/block
			}
        	queue_push(hostQueue, domain);
			pthread_mutex_unlock(buffMutex); //Unlock/block the thread
			done = true; //hostname was pushed thru successfully
    	}
		done = false; //Reset push for next hostname
	}
    return NULL;
}

void* resolveThread(void* id){
	struct thread* thread = id; //Make a thread to hold info
   	char* domain; //domain char arrays 
	FILE* outFile = thread->threadFile; //Output file for results
	pthread_mutex_t* buffMutex = thread->buffMutex; //Buffer mutex
	pthread_mutex_t* outMutex = thread->outMutex; //Output mutex
	queue* hostQueue = thread->hostQueue; //Queue
	char IPAdd[MAX_IP_LENGTH]; //IP Addresses with length limit, array
    while(!queue_is_empty(hostQueue) || requests_exist){ //loop while the queue is not empty and requestThreads exist
		pthread_mutex_lock(buffMutex); //lock/block  buffer
		domain = queue_pop(hostQueue); //pop from queue
		if(domain == NULL){ //if empty/no domain, unlock
			pthread_mutex_unlock(buffMutex);
			usleep(rand() % 100 + 5);//Wait 0-100 microseconds
		}		
		else { //Unlock/block and continue
			pthread_mutex_unlock(buffMutex);
			if(dnslookup(domain, IPAdd, sizeof(IPAdd)) == UTIL_FAILURE)//look up domain
				strncpy(IPAdd, "", sizeof(IPAdd));
			printf("%s:%s\n", domain, IPAdd); //Print Domain/host with IP Address
            pthread_mutex_lock(outMutex); //lock output file
			fprintf(outFile, "%s,%s\n", domain, IPAdd); //write to output file same as above print statement
			pthread_mutex_unlock(outMutex); //unlock output
    	}
			free(domain);//deallocates the memory (domain)  previously allocated by malloc
	}
    return NULL;
}

int main(int argc, char* argv[]){

    	/* Local Vars */
        queue hostQueue; //Hostname queue
        int inputFiles = argc-2; //Number of arguments passed
        FILE* outFile   = NULL; //output pointer for out file
        int nCPU        = sysconf( _SC_NPROCESSORS_ONLN );
        FILE* inputfps[inputFiles]; //Array of inputs
        int resolveThreads;

   	/* Check Arguments */
   	 if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
    	}

	/*Set amount of threads */
	if (nCPU < MAX_RESOLVE_THREADS)
		resolveThreads = nCPU;
	else if (nCPU < MIN_RESOLVE_THREADS)
		resolveThreads = MIN_RESOLVE_THREADS;
	else 
		resolveThreads = MAX_RESOLVE_THREADS;
	
	/*Arrays of threads for requests and resolves */
	pthread_t requests[inputFiles];
	pthread_t resolves[resolveThreads];
	
	/*Mutexes for hostQueue and output file*/
	pthread_mutex_t buffMutex;
	pthread_mutex_t outMutex;
	struct thread requestInfo[inputFiles];
	struct thread resolveInfo[resolveThreads];

	printf("Numver of Requests: %d\n", inputFiles);
	printf("Number of Resolveds: %d\n\n", resolveThreads);
	
	/* Limit check for number of files, 10 max */
	if((inputFiles)>MAX_INPUT_FILES){ 
		fprintf(stderr, "Max number of files allowed is %d\n", MAX_INPUT_FILES);
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
	}
	
	/*Open Output File */
	outFile = fopen(argv[argc-1], "w"); //Open output/results file
	if(!outFile){
		perror("Error Opening to the Output File");
		return EXIT_FAILURE;
	}			
	
	/*Loop Through Input Files */
	int i;
	for(i=1; i<(argc-1); i++){ 	//Open input files up to 3
		inputfps[i-1] = fopen(argv[i], "r");
	}
	
	/*Initialize  Queue */
	queue_init(&hostQueue, MAX_INPUT_FILES); 
	
	/* Initialize mutexes */
	pthread_mutex_init(&buffMutex, NULL);
	pthread_mutex_init(&outMutex, NULL);

	/*Create request pthreads*/
	for(i=0; i<inputFiles; i++){ //Iterate thru input files
		//Set data for struct to pass to current pthread
		requestInfo[i].threadFile = inputfps[i];
		requestInfo[i].buffMutex   = &buffMutex;
		requestInfo[i].outMutex    = NULL;
		requestInfo[i].hostQueue      = &hostQueue;		
		//pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); arg==id passed to request
        pthread_create(&(requests[i]), NULL, requestThread, &(requestInfo[i])); 
		printf("Create request thread %d\n", i);
	}
	
	/*Create resolve pthreads*/
	for(i=0; i<resolveThreads; i++){
		resolveInfo[i].threadFile = outFile;
		resolveInfo[i].buffMutex   = &buffMutex;
		resolveInfo[i].outMutex    = &outMutex;
		resolveInfo[i].hostQueue      = &hostQueue;
		pthread_create(&(resolves[i]), NULL, resolveThread, &(resolveInfo[i]));
		printf("Create resolve thread %d\n", i);
	}		

	/*Wait for Request Threads to complete*/
	for(i=0; i<inputFiles; i++){
        	pthread_join(requests[i], NULL);
			printf("Requested %d \n", i);
	}	
	requests_exist=false;
	
	/*Wait for Resolve Threads to complete*/	
	for(i=0; i<resolveThreads; i++){
        pthread_join(resolves[i], NULL); // int pthread_join(pthread_t thread, void **retval); joins with a termindated thread
		printf("Resolved %d \n", i);
	}
	
	/*Clean up the Queue*/
	queue_cleanup(&hostQueue);
	
	/*Close the Output file */
	fclose(outFile);
	
	/*Close the Input Files */
	for(i=0; i<inputFiles; i++){
		fclose(inputfps[i]);
	}
	
	/*Destroy Mutexes*/
	pthread_mutex_destroy(&buffMutex);
	pthread_mutex_destroy(&outMutex);
    return 0;
}
