#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0
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
    int n_rqts_processed;
    int cur_request;
}requests;

/**
 * @brief 
 * 
 * @param requests 
 */
void* calculate_pi(void* rqts){
    requests* rqts_pt = (requests*)rqts;
    FILE* fp = fopen(rqts_pt->name, "a");
    int result;
    if(fp != NULL){
        usleep(rqts_pt->time_waiting);
        result = rqts_pt->digits_pi * rqts_pt->time_waiting;
        fprintf(fp, "requisição %d: digits(%d) * time_waiting(%d) results %d\n", rqts_pt->cur_request, rqts_pt->digits_pi, rqts_pt->time_waiting, result);
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    fclose(fp);
    return NULL;
}
int check_thread_is_free(pthread_t threads[]){
    int i = 0;
    int status;
    for(int i; i < N_WTHREADS;){
        status = pthread_kill(threads[i], 0);
        if (status == 0) {
            printf("Thread is occupied.\n");
            i++;
        }else if (status == ESRCH) {
            printf("Thread is free.\n");
            return i;
        }
    }
    if(i == N_WTHREADS)
        check_thread_is_free(threads);
}   
/**
 * @brief 
 * 
 * @param fp 
 */
void* dispatcher_thread_function(void *fp){
    pthread_t threads[N_WTHREADS];
    FILE *requests_file = fp;
    int ct_threads = 0, flag = TRUE, digits = 0, time_waiting = 0;
    int cur_processed_rqt = 0;//counter that will indicate the number of the current
    //request to be read 
    requests* rqts_pt = malloc(sizeof(requests));
    rqts_pt->name = malloc(20*sizeof(char));
    if(requests_file != NULL){
        while(fscanf(fp, "%d;%d", &digits, &time_waiting) != EOF){
            if((ct_threads < N_WTHREADS) && (flag == TRUE)){
                sprintf(rqts_pt->name, "%s%d%s", "thread", ct_threads, ".txt");
                rqts_pt->cur_request = cur_processed_rqt;
                rqts_pt->digits_pi=digits;
                rqts_pt->time_waiting = time_waiting;
                if(pthread_create(&threads[ct_threads], NULL, calculate_pi, (void*)rqts_pt) != 0){
                    perror("-1");
                }
                if(pthread_join(threads[ct_threads], NULL) != 0){
                    return (void*)-1;
                }  
                ct_threads++;
                cur_processed_rqt++;
                usleep(time_request);
                //printf("%d %d\n", digits, time_waiting);
            }else{
                flag = FALSE;
                ct_threads = check_thread_is_free(threads);
                sprintf(rqts_pt->name, "%s%d%s", "thread", ct_threads, ".txt");
                rqts_pt->digits_pi=digits;
                rqts_pt->time_waiting = time_waiting;
                rqts_pt->cur_request = cur_processed_rqt;
                if(pthread_create(&threads[ct_threads], NULL, calculate_pi, (void*)rqts_pt) != 0){
                    perror("-1");
                }
                if(pthread_join(threads[ct_threads], NULL) != 0){
                    return (void*)-1;
                }
                cur_processed_rqt++;  
                usleep(time_request);
                //printf("%d %d\n", rqts_pt->digits_pi, rqts_pt->time_waiting);
            }
        }
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    free(rqts_pt->name);
    free(rqts_pt);
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
    fclose(fp);
    return 0;
}
