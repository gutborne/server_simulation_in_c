#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <gmp.h>
#define TRUE 1
#define FALSE 0
#define n_requests 1000 //number of requests
#define time_request 100000//100000 microseconds = 100 milliseconds
#define N_WTHREADS  5 //constant of the program that indicates the number of
//workers threads
 

/**
 * @brief 
 * 
 */
typedef struct requests_{
    int thread_id;
    char* name;
    int digits_pi;
    int time_waiting;
    int* n_rqts_processed;
    int cur_request;
}requests;






/**
 * @brief 
 * 
 * @param rqts 
 * @return void* 
 */
void* calculate_pi(void* rqts){
    requests* rqts_pt = (requests*)rqts;
    usleep(rqts_pt->time_waiting * 100);
    FILE* fp = fopen(rqts_pt->name, "a");
    mpz_t num_z, den_z;
    mpf_t pi, numerator, denominator; 
    mpf_set_default_prec(rqts_pt->digits_pi * 3);
    mpz_inits(num_z, den_z, NULL);
    mpf_inits(pi, numerator, denominator, NULL);
    mpz_set_si(num_z, 22);
    mpz_set_si(den_z, 7);
    mpf_set_z(numerator, num_z);
    mpf_set_z(denominator, den_z);
    if(fp != NULL){
        mpf_div(pi, numerator, denominator);
        rqts_pt->n_rqts_processed[rqts_pt->thread_id]++;
        gmp_fprintf(fp, "rqts %d: Pi with %d digits = %.Ff \n", rqts_pt->cur_request, rqts_pt->digits_pi, pi);
        mpz_clears(num_z, den_z, NULL);
        mpf_clears(numerator, denominator, pi, NULL);
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


void initialize_with_zero(int* ptr){
    for(int i = 0; i < N_WTHREADS; i++)
        ptr[i] = 0;
}

void print_n_rqts_for_file(int* ptr_n_rqts_processed){
    FILE* file_ptr = NULL;
    char thread_name[20];
    for(int i = 0; i < N_WTHREADS; i++){
        sprintf(thread_name, "%s%d%s", "thread", i, ".txt");
        file_ptr = fopen(thread_name, "a");
        if(file_ptr != NULL){
            fprintf(file_ptr, "thread %d = %d requests processed\n", i, ptr_n_rqts_processed[i]);
            fclose(file_ptr);
        }
    }
    
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
    //
    int *ptr_n_rqts_processed = malloc(sizeof(int) * N_WTHREADS);
    initialize_with_zero(ptr_n_rqts_processed);
    rqts_pt->n_rqts_processed = ptr_n_rqts_processed;
    //
    if(requests_file != NULL){
        while(fscanf(fp, "%d;%d", &digits, &time_waiting) != EOF){
            if((ct_threads < N_WTHREADS) && (flag == TRUE)){
                sprintf(rqts_pt->name, "%s%d%s", "thread", ct_threads, ".txt");
                rqts_pt->cur_request = cur_processed_rqt;
                rqts_pt->digits_pi=digits;
                rqts_pt->time_waiting = time_waiting;
                rqts_pt->thread_id = ct_threads; 
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
                rqts_pt->thread_id = c;
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
        print_n_rqts_for_file(ptr_n_rqts_processed);

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



/*
void* calculate_pi(void* rqts){
    requests* rqts_pt = (requests*)rqts;
    FILE* fp = fopen(rqts_pt->name, "a");
    mpz_t num, den;
    mpf_t result, denominator;
    mpz_inits(num, den, NULL);
    mpf_inits(result, denominator, NULL);
    mpf_set_default_prec(rqts_pt->digits_pi * 4); // Multiply by 4 for sufficient precision

    // Set the numerator and denominator
    mpz_set_si(num, 22);
    mpz_set_si(den, 7);
    if(fp != NULL){
        // Perform the division
        mpf_set_z(result, num);
        mpf_set_z(denominator, den);
        mpf_div(result, result, denominator);

        // Convert the result to a string with the desired precision
        char* resultStr = (char*)malloc((rqts_pt->digits_pi + 2) * sizeof(char));
        mp_exp_t exp;
        mpf_get_str(resultStr, &exp, 10, rqts_pt->digits_pi, result);
        
        //gmp_fprintf(fp, "rqts %d: Pi with %d digits = %.*Ff | time_waiting -> %d \n", rqts_pt->cur_request, rqts_pt->digits_pi, pi, rqts_pt->time_waiting);
        fprintf(fp, "rqts %d: Pi with %d digits = %s \n", rqts_pt->cur_request, rqts_pt->digits_pi, resultStr);
        mpz_clears(num, den, NULL);
        mpf_clear(result);
        free(resultStr);
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    fclose(fp);
    return NULL;
}
*/
/**
 * @brief 
 * 
 * @param requests 
 */
/*
void* calculate_pi(void* rqts){
    requests* rqts_pt = (requests*)rqts;
    FILE* fp = fopen(rqts_pt->name, "a");
    int result;
    //double pi = 0.0;
    //int sign = 1;
    int i;
    mpf_t term, pi;
    mpf_inits(term, pi);


    if(fp != NULL){
        for (i = 0; i < rqts_pt->digits_pi; ++i) {
            double term = 1.0 / (2 * i + 1);
            pi += sign * term;
            sign *= -1;
        }
        pi *= 4;
       // printf("\n", rqts_pt->digits_pi, pi);
        usleep(rqts_pt->time_waiting * 1000);
        gmp_fprintf(fp, "request %d: Pi with %d digits = %f | time_waiting -> %d \n", rqts_pt->cur_request, rqts_pt->digits_pi, pi, rqts_pt->time_waiting);
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    fclose(fp);
    return NULL;
}*/
