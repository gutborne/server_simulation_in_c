#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define n_requests 10 //number of requests
#define time_request 100000//100000 microseconds = 100 milliseconds
#define N_WTHREADS  5 //constant of the program that indicates the number of
//workers threads
 

/**
 * @brief 
 * 
 */
typedef struct requests_{
    char* name;
    int digits_pi;
    int time_waiting;
}requests;

/**
 * @brief 
 * 
 * @param requests 
 */
void* calculate_pi(void* rqts){
    requests* rqts_pt = (requests*)rqts;    

}   
/**
 * @brief 
 * 
 * @param fp 
 */
void* dispatcher_thread_function(void *fp){
    pthread_t threads[N_WTHREADS];
    FILE *requests_file = fp;
    int digits, time_waiting, cont = n_requests, i = 0;
    requests* rqts_pt = malloc(sizeof(requests));
    if(requests_file != NULL){
        while(fscanf(fp, "%d;%d", rqts_pt->digits_pi, rqts_pt->time_waiting) != EOF){
            usleep(time_request);
            if(pthread_create(threads + i, NULL, calculate_pi, (void*)rqts_pt) != 0){
                perror(-1);
            }
            if(pthread_join(threads +  i, NULL) != 0){
                return -1;
            }  
            cont--;
            i++;
            printf("%d %d cont = %d\n", digits, time_waiting, cont);    
        }
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    pthread_exit(NULL);
    return NULL;
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
int main(){
    srand(time(0));
    FILE *fp = fopen("requests.txt", "w");
    if(fp != NULL){
        int digits = 0, time_waiting = 0;
        for(int i = 0; i < n_requests; i++){
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
