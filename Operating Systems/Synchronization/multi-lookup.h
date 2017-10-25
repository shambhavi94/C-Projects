//Shambhavi Srivastava

#include <pthread.h>


#define MAX_INPUT_FILES 10
#define MAX_RESOLVE_THREADS 10
#define MIN_RESOLVE_THREADS 2
#define MAX_NAME_LENGTHS 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
typedef int bool; //for readability
#define true 1
#define false 0
bool requests_exist = true;

//Threads
struct thread{
	queue* hostQueue;
    FILE* threadFile;
    pthread_mutex_t* buffMutex;
    pthread_mutex_t* outMutex;
};
