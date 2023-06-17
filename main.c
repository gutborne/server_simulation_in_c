#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#define TRUE 1
#define FALSE 0
#define n_requests 20 //number of requests
#define time_request 100000//100000 microseconds = 100 milliseconds
#define N_WTHREADS  2 //constant of the program that indicates the number of
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
        usleep(rqts_pt->time_waiting * 1000);
        result = rqts_pt->digits_pi * rqts_pt->time_waiting;
        fprintf(fp, "requisição %d: digits(%d) * time_waiting(%d) results %d\n", rqts_pt->cur_request, rqts_pt->digits_pi, rqts_pt->time_waiting, result);
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    fclose(fp);
    return NULL;
}
int check_thread_is_free(pthread_t threads[]){
    int status_thread;
    int flag = TRUE;
    while(flag){
        int index_thread = (rand() % ((N_WTHREADS - 1) - 0 + 1));
        status_thread = pthread_kill(threads[index_thread], 0);
        if (status_thread == 0) {
            printf("Thread %d is occupied.\n", index_thread);
        }else if (status_thread == ESRCH) {
            printf("Thread %d is free.\n", index_thread);
            return index_thread;
        }
    }
}

/**
 * @brief the aim of this function is clean the requests of the worker thread files
 * if they were already created in previous executions of this program in order to get
 * rid of these previous ones and maintain just the current ones.
 */
void clean_files_wthreads(){
    int counter = 0;
    char* thread_name = malloc(20*sizeof(char));
    FILE* fp;
    for(counter; counter < N_WTHREADS;){
        sprintf(thread_name, "%s%d%s", "thread", counter, ".txt");
        fp = fopen(thread_name, "w");
        if(fp != NULL){
            counter++;
        }else{
            printf("ERROR IN OPEN THE FILE!");
        }
    }
    fclose(fp);
}

/**
 * @brief 
 * 
 * @param fp 
 */
void* dispatcher_thread_function(void *fp){
    pthread_t worker_threads[N_WTHREADS];
    FILE *requests_file = fp;
    int ct_threads = 0, flag = TRUE, digits = 0, time_waiting = 0, c = 0;
    int cur_processed_rqt = 0;//counter that will indicate the number of the current
    //request to be read 
    requests* rqts_pt = malloc(sizeof(requests));
    rqts_pt->name = malloc(20*sizeof(char));
    clean_files_wthreads();
    if(requests_file != NULL){
        while(fscanf(fp, "%d;%d", &digits, &time_waiting) != EOF){
            if((ct_threads < N_WTHREADS) && (flag == TRUE)){
                sprintf(rqts_pt->name, "%s%d%s", "thread", ct_threads, ".txt");
                rqts_pt->cur_request = cur_processed_rqt;
                rqts_pt->digits_pi=digits;
                rqts_pt->time_waiting = time_waiting;
                if(pthread_create(&worker_threads[ct_threads], NULL, calculate_pi, (void*)rqts_pt) != 0){
                    perror("-1");
                }
                pthread_join(worker_threads[ct_threads], NULL);
                ct_threads++;
                //printf("%d %d\n", digits, time_waiting);
            }else{
                flag = FALSE;
                c = check_thread_is_free(worker_threads);
                sprintf(rqts_pt->name, "%s%d%s", "thread", c, ".txt");
                rqts_pt->digits_pi=digits;
                rqts_pt->time_waiting = time_waiting;
                rqts_pt->cur_request = cur_processed_rqt;
                if(pthread_create(&worker_threads[c], NULL, calculate_pi, (void*)rqts_pt) != 0){
                    perror("-1");
                }
                /*if(cur_processed_rqt == n_requests - 1){
                    pthread_join(worker_threads[c], NULL);
                }*/
            }
            cur_processed_rqt++;
            usleep(time_request);
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
    pthread_exit(NULL);
}

/**
 * @brief This main function will generate the file with the
 * requests for the server
 * @return int 
 */
int main(){
    srand(time(NULL));
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
