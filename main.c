#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define N 10; //constant of the program that indicate the number of
//workers threads

struct requests_{
    int digit_pi;
    int time_waiting;
}requests;


/**
 * * @brief this function will receive the requests, instantiate the worker threads and
 * the dispatcher thread to help the server to process the requests of the clients.
 * @param fp 
 * @param n_threads 
 */
void server(FILE *fp, int n_threads){

}

/**
 * @brief This main function will generate the file with the
 * requests for the server
 * @return int 
 */
int main(){
    srand(time(0));
    FILE *fp = fopen("requests.txt", "w");
    int num = 0;
    if(fp != NULL){
        int digits = 0, time_waiting = 0;
        for(int i = 0; i < 10; i++){
            digits = (rand() % (100 - 10 + 1)) + 10;//PI's numbers of digits 
            time_waiting = (rand() % (1500 - 500 + 1)) + 500;//time of waiting
            fprintf(fp, "%d;%d\n", digits, time_waiting);
        }
        fclose(fp);
        fp = fopen("requests.txt", "r");
        if(fp != NULL){
            FILE *fp2 = fopen("requests2.txt", "w");
            while(fscanf(fp, "%d;%d", &digits, &time_waiting) != EOF){
                fprintf(fp2, "%d %d\n", digits, time_waiting);    
            }
        }else{
            printf("ERROR IN OPEN THE FILE!");
        }
        fclose(fp);
    }else{
        printf("ERROR IN OPEN THE FILE!");
    }
    return 0;
}
