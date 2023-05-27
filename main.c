#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 #define N_WTHREADS  10 //constant of the program that indicate the number of
//workers threads

/**
 * @brief 
 * 
 */
struct requests_{
    int digit_pi;
    int time_waiting;
}requests;

/**
 * @brief 
 * 
 * @param fp 
 */
void* dispatcher_thread_function(void *fp){
    pthread_t threads[N_WTHREADS];
    FILE *requests_file = fp;
    int digits, time_waiting;
    if(requests_file != NULL){
        while(fscanf(fp, "%d;%d", &digits, &time_waiting) != EOF){
            printf("%d %d\n", digits, time_waiting);    
        }
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    pthread_exit(NULL);
}


/**
 * * @brief this function will receive the requests, instantiate the worker threads and
 * the dispatcher thread to help the server to process the requests of the clients.
 * @param fp //pointer to the archive
 * @param n_threads //number of worker threads
 */
void server(FILE *fp){
    int r = 0;
    pthread_t dispatcher;
    r = pthread_create(&dispatcher, NULL, dispatcher_thread_function, (void*)fp);
    if(r){
        printf("ERROR IN THE CREATION OF THE THREAD!\n");//if r is different than 0
        exit(-1);
    }else{
        printf("DISPATCHER THREAD CREATED WITH SUCCESS!\n");//if r is equal to 0
    }
    pthread_join(dispatcher, NULL);
}

/**
 * @brief This main function will generate the file with the
 * requests for the server
 * @return int 
 */
//inicio: 0x7ff8a890fa90
int main(){
    srand(time(0));
    FILE *fp = fopen("requests.txt", "w");
    if(fp != NULL){
        int digits = 0, time_waiting = 0;
        for(int i = 0; i < 100; i++){
            digits = (rand() % (100 - 10 + 1)) + 10;//PI's numbers of digits 
            time_waiting = (rand() % (1500 - 500 + 1)) + 500;//time of waiting
            fprintf(fp, "%d;%d\n", digits, time_waiting);
        }

        fclose(fp);
        fp = fopen("requests.txt", "r");
        if(fp != NULL)
            server(fp);//call the server function
        else
            printf("ERROR IN OPEN THE FILE!");
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    return 0;
}
