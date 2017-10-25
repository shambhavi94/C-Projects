#Shambhavi Srivastava

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Include Flags*/
#define _GNU_SOURCE
#define DEFAULT_ITERATIONS 100000
#define RADIUS (RAND_MAX / 2) //from pi-sched
#define MAXFILENAMELENGTH 80 //from rw
#define DEFAULT_INPUTFILENAME "rwinput" //from rw
#define DEFAULT_OUTPUTFILENAMEBASE "rwoutput" //from rw
#define DEFAULT_BLOCKSIZE 1024 //from rw
#define DEFAULT_TRANSFERSIZE 1024*100 //rw

//from pi-sched
static inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

//from pi-sched
static inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

int main(int argc, char* argv[]){
    struct sched_param param;
    /* Comment - Variable Declarations */
    pid_t pid, wpid;
    int policy = SCHED_OTHER;
    int numProcesseses = 10;
    int status;
    int p=0;

    //from pi-sched
    long i = DEFAULT_ITERATIONS;
    long iterations = DEFAULT_ITERATIONS;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;

    //from rw
    int rv;
    int inputFD;
    int outputFD;
    char inputFilename[MAXFILENAMELENGTH];
    char outputFilename[MAXFILENAMELENGTH];
    char outputFilenameBase[MAXFILENAMELENGTH];

    ssize_t transfersize = 0;
    ssize_t blocksize = 0;
    char* transferBuffer = NULL;
    ssize_t buffersize;

    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    int totalReads = 0;
    ssize_t bytesWritten = 0;
    ssize_t totalBytesWritten = 0;
    int totalWrites = 0;
    int inputFileResets = 0;

    /* Set i/o files */
    strncpy(inputFilename, DEFAULT_INPUTFILENAME, MAXFILENAMELENGTH);
    strncpy(outputFilenameBase, DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH);

    //from pi-sched
    /* Set default policy if not supplied */
    if(argc < 2){
      policy = SCHED_OTHER;
    }

    /* Set policy if supplied */
    if(argc > 1){
    if(!strcmp(argv[1], "SCHED_OTHER")){
        policy = SCHED_OTHER;
    }
    else if(!strcmp(argv[1], "SCHED_FIFO")){
        policy = SCHED_FIFO;
    }
    else if(!strcmp(argv[1], "SCHED_RR")){
        policy = SCHED_RR;
    }
    else{
        fprintf(stderr, "Invalid Scheduling Policy\n");
        exit(EXIT_FAILURE);
    }
    }

    /* Set # of Processes */
    if(argc > 2){
        if(!strcmp(argv[2], "SMALL")){
            numProcesseses = 5;
        }
        else if(!strcmp(argv[2], "MEDIUM")){
            numProcesseses = 20;
        }
        else if(!strcmp(argv[2], "LARGE")){
            numProcesseses = 100;
        }
        else{
            fprintf(stderr, "Invalid number of procceses\n");
            exit(EXIT_FAILURE);
        }
    }

    //from pi-sched
    /* Set process to max prioty for given scheduler */
    param.sched_priority = sched_get_priority_max(policy);

    //from pi-sched
    /* Set new scheduler policy */
    fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
    fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
    if(sched_setscheduler(0, policy, &param)){
      perror("Error setting scheduler policy");
      exit(EXIT_FAILURE);
    }
    fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

    /* Do for number of processes, MIXED Processes*/
    if (!strcmp(argv[3], "MIXED")){
        printf("%d processes forked \n", numProcesseses);
        for(i = 0; i < numProcesseses; i++){
            //forking failed
            if((pid = fork())==-1){
                fprintf(stderr, "Error Forking Child Process");
                exit(EXIT_FAILURE);
            }
            //Child process, forking succeeds
            if(pid == 0){
                //from pi-sched
                /* Calculate pi using statistical methode across all iterations*/
                for(i=0; i < iterations; i++){
                x = (random() % (RADIUS * 2)) - RADIUS;
                y = (random() % (RADIUS * 2)) - RADIUS;
                if(zeroDist(x,y) < RADIUS){
                   inCircle++;
                }
                inSquare++;
                }

                /* Finish calculation */
                pCircle = inCircle/inSquare;
                piCalc = pCircle * 4.0;

                /* Print result */
                fprintf(stdout, "pi = %f\n", piCalc);

                //from rw
                /* Confirm blocksize is multiple of and less than transfersize*/
                if(blocksize > transfersize){
                fprintf(stderr, "blocksize can not exceed transfersize\n");
                exit(EXIT_FAILURE);
                }
                if(transfersize % blocksize){
                fprintf(stderr, "blocksize must be multiple of transfersize\n");
                exit(EXIT_FAILURE);
                }

                /* Allocate buffer space */
                buffersize = blocksize;
                if(!(transferBuffer = malloc(buffersize*sizeof(*transferBuffer)))){
                perror("Failed to allocate transfer buffer");
                exit(EXIT_FAILURE);
                }

                /* Open Input File Descriptor in Read Only mode */
                if((inputFD = open(inputFilename, O_RDONLY | O_SYNC)) < 0){
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
                }

                /* Open Output File Descriptor in Write Only mode with standard permissions*/
                rv = snprintf(outputFilename, MAXFILENAMELENGTH, "%s-%d",
                    outputFilenameBase, getpid());
                if(rv > MAXFILENAMELENGTH){
                fprintf(stderr, "Output filenmae length exceeds limit of %d characters.\n",
                    MAXFILENAMELENGTH);
                exit(EXIT_FAILURE);
                }
                else if(rv < 0){
                perror("Failed to generate output filename");
                exit(EXIT_FAILURE);
                }
                if((outputFD =
                open(outputFilename,
                    O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
                perror("Failed to open output file");
                exit(EXIT_FAILURE);
                }

                /* Print Status */
                fprintf(stdout, "Reading from %s and writing to %s\n",
                    inputFilename, outputFilename);

                /* Read from input file and write to output file*/
                do{
                /* Read transfersize bytes from input file*/
                bytesRead = read(inputFD, transferBuffer, buffersize);
                if(bytesRead < 0){
                    perror("Error reading input file");
                    exit(EXIT_FAILURE);
                }
                else{
                    totalBytesRead += bytesRead;
                    totalReads++;
                }

                /* If all bytes were read, write to output file*/
                if(bytesRead == blocksize){
                    bytesWritten = write(outputFD, transferBuffer, bytesRead);
                    if(bytesWritten < 0){
                    perror("Error writing output file");
                    exit(EXIT_FAILURE);
                    }
                    else{
                    totalBytesWritten += bytesWritten;
                    totalWrites++;
                    }
                }

                /* Otherwise assume we have reached the end of the input file and reset */
                else{
                    if(lseek(inputFD, 0, SEEK_SET)){
                    perror("Error resetting to beginning of file");
                    exit(EXIT_FAILURE);
                    }
                    inputFileResets++;
                }

                }while(totalBytesWritten < transfersize);

                /* Output some possibly helpfull info to make it seem like we were doing stuff */
                fprintf(stdout, "Read:    %zd bytes in %d reads\n",
                    totalBytesRead, totalReads);
                fprintf(stdout, "Written: %zd bytes in %d writes\n",
                    totalBytesWritten, totalWrites);
                fprintf(stdout, "Read input file in %d pass%s\n",
                    (inputFileResets + 1), (inputFileResets ? "es" : ""));
                fprintf(stdout, "Processed %zd bytes in blocks of %zd bytes\n",
                    transfersize, blocksize);

                /* Free Buffer */
                free(transferBuffer);

                /* Close Output File Descriptor */
                if(close(outputFD)){
                perror("Failed to close output file");
                exit(EXIT_FAILURE);
                }

                /* Close Input File Descriptor */
                if(close(inputFD)){
                perror("Failed to close input file");
                exit(EXIT_FAILURE);
                }
                exit(0);
            }
        }

        //Parent must wait for childrent to terminate
        while ((wpid = wait(&status)) > 0) {
            //normal process termination
            if (WIFEXITED(status)) {
                printf("Exit of child %d was normal \n", wpid);
                p++;
            }
        }
        printf("Total # of Processes terminated was %d\n", p);
            return EXIT_SUCCESS;
            }

    //IO-Bound processes w/ given number of processes
    else if (!strcmp(argv[3], "IO")){
        printf("%d processes forked \n", numProcesseses);
        for(i = 0; i < numProcesseses; i++){
            //Failed Fork process
            if((pid = fork())==-1){
                fprintf(stderr, "Error Forking Child Process");
                exit(EXIT_FAILURE);
            }
            //Sucess Fork process, children produced
            if(pid == 0){

                //from rw
                /* Confirm blocksize is multiple of and less than transfersize*/
                if(blocksize > transfersize){
                fprintf(stderr, "blocksize can not exceed transfersize\n");
                exit(EXIT_FAILURE);
                }
                if(transfersize % blocksize){
                fprintf(stderr, "blocksize must be multiple of transfersize\n");
                exit(EXIT_FAILURE);
                }

                /* Allocate buffer space */
                buffersize = blocksize;
                if(!(transferBuffer = malloc(buffersize*sizeof(*transferBuffer)))){
                perror("Failed to allocate transfer buffer");
                exit(EXIT_FAILURE);
                }

                /* Open Input File Descriptor in Read Only mode */
                if((inputFD = open(inputFilename, O_RDONLY | O_SYNC)) < 0){
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
                }

                /* Open Output File Descriptor in Write Only mode with standard permissions*/
                rv = snprintf(outputFilename, MAXFILENAMELENGTH, "%s-%d",
                    outputFilenameBase, getpid());
                if(rv > MAXFILENAMELENGTH){
                fprintf(stderr, "Output filenmae length exceeds limit of %d characters.\n",
                    MAXFILENAMELENGTH);
                exit(EXIT_FAILURE);
                }
                else if(rv < 0){
                perror("Failed to generate output filename");
                exit(EXIT_FAILURE);
                }
                if((outputFD =
                open(outputFilename,
                    O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
                perror("Failed to open output file");
                exit(EXIT_FAILURE);
                }

                /* Print Status */
                fprintf(stdout, "Reading from %s and writing to %s\n",
                    inputFilename, outputFilename);

                /* Read from input file and write to output file*/
                do{
                /* Read transfersize bytes from input file*/
                bytesRead = read(inputFD, transferBuffer, buffersize);
                if(bytesRead < 0){
                    perror("Error reading input file");
                    exit(EXIT_FAILURE);
                }
                else{
                    totalBytesRead += bytesRead;
                    totalReads++;
                }

                /* If all bytes were read, write to output file*/
                if(bytesRead == blocksize){
                    bytesWritten = write(outputFD, transferBuffer, bytesRead);
                    if(bytesWritten < 0){
                    perror("Error writing output file");
                    exit(EXIT_FAILURE);
                    }
                    else{
                    totalBytesWritten += bytesWritten;
                    totalWrites++;
                    }
                }

                /* Otherwise assume we have reached the end of the input file and reset */
                else{
                    if(lseek(inputFD, 0, SEEK_SET)){
                    perror("Error resetting to beginning of file");
                    exit(EXIT_FAILURE);
                    }
                    inputFileResets++;
                }

                }while(totalBytesWritten < transfersize);

                /* Output some possibly helpfull info to make it seem like we were doing stuff */
                fprintf(stdout, "Read:    %zd bytes in %d reads\n",
                    totalBytesRead, totalReads);
                fprintf(stdout, "Written: %zd bytes in %d writes\n",
                    totalBytesWritten, totalWrites);
                fprintf(stdout, "Read input file in %d pass%s\n",
                    (inputFileResets + 1), (inputFileResets ? "es" : ""));
                fprintf(stdout, "Processed %zd bytes in blocks of %zd bytes\n",
                    transfersize, blocksize);

                /* Free Buffer */
                free(transferBuffer);

                /* Close Output File Descriptor */
                if(close(outputFD)){
                perror("Failed to close output file");
                exit(EXIT_FAILURE);
                }

                /* Close Input File Descriptor */
                if(close(inputFD)){
                perror("Failed to close input file");
                exit(EXIT_FAILURE);
                }
                exit(0);
            }
        }
        //child exits
        while ((wpid = wait(&status)) > 0) {
            if (WIFEXITED(status)) {
                printf("Exit of child %d was normal \n", wpid);
                p++;
            }
        }
        printf("Total # of Processes terminated was %d\n", p);
            return EXIT_SUCCESS;
            }
    //CPU-BOUND PROCESS
    else {
        printf("%d processes forked \n", numProcesseses);
        for(i = 0; i < numProcesseses; i++){
            //forking fails
            if((pid = fork())==-1){
                fprintf(stderr, "Error Forking Child Process");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                /* Calculate pi using statistical methode across all iterations*/
                for(i=0; i<iterations; i++){
                x = (random() % (RADIUS * 2)) - RADIUS;
                y = (random() % (RADIUS * 2)) - RADIUS;
                if(zeroDist(x,y) < RADIUS){
                   inCircle++;
                }
                inSquare++;
                }

                /* Finish calculation */
                pCircle = inCircle/inSquare;
                piCalc = pCircle * 4.0;

                /* Print Result */
                fprintf(stdout, "pi = %f\n", piCalc);

                exit(0);
            }
        }

        while ((wpid = wait(&status)) > 0) {
            if (WIFEXITED(status)) {
                printf("Exit of child %d was normal \n", wpid);
                p++;
            }
        }
        printf ("Total Num of Proc terminated was %d\n", p);
        return EXIT_SUCCESS;
    }
}
