/*
 *
 * Some of the logic of this code was inspired by the example code provided for use in the assignment. Here is a link to that code:
 *
 * https://replit.com/@cs344/65prodconspipelinec#main.c
 *
 * */

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Setting up macros for my error return values */
#define THREAD_CREATE_ERROR 1
#define THREAD_JOIN_ERROR 2

#define BUFFER_SIZE 1001
#define NUM_ITEMS_PER_LINE 80 // this value is the number of items that should be outputted per line. It is less than the buffer size, allowing us to model the buffer as being unbounded.

/* Buffer 1 Globals */
char buffer_1[BUFFER_SIZE];
int count_1 = 0; // number of items in the buffer
int producer_index_1 = 0; // index where the input thread will put the next item
int consumer_index_1 = 0; // index where the linear separator thread will put the next item
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER; // creating the mutex for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER; // conditional variable for buffer 1

/* Buffer 2 Globals */
char buffer_2[BUFFER_SIZE];
int count_2 = 0; // number of items in the buffer
int producer_index_2 = 0; // index where the producer thread will put the next item
int consumer_index_2 = 0; // index where the consumer thread will put the next item
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER; // creating the mutex for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER; // conditional variable for buffer 2

/* Buffer 3 Globals */
char buffer_3[BUFFER_SIZE];
int count_3 = 0; // number of items in the buffer
int producer_index_3 = 0; // index where the producer thread will put the next item
int consumer_index_3 = 0; // index where the consumer thread will put the next item
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER; // creating the mutex for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER; // conditional variable for buffer 3


bool shouldStopInput = false;

void putInBuffer(char item, char* buffer, pthread_mutex_t* mutex, pthread_cond_t* full_t, int* count, int* producer_index) {
    pthread_mutex_lock(mutex); // locking the mutex before writing to the buffer
    buffer[(*producer_index)++] = item; // adding the char to the buffer and iterating producer_index
    *count += 1; // iterating the amount of items in buffer
    pthread_cond_signal(full_t); // telling the consumer that the buffer is no longer empty
    pthread_mutex_unlock(mutex); // unlocking the mutex to signal that the lineSeparator thread can process this data
} // end of "putInBuffer1" function


char getBuffer(const char* buffer, pthread_mutex_t* mutex, pthread_cond_t* full_t, int* count, int* consumer_index) {
    pthread_mutex_lock(mutex); // locking the mutex before writing to the buffer
    while (*count == 0) {
        pthread_cond_wait(full_t, mutex); // the buffer is empty, based on the condition of the loop, so wait until the buffer has data
    }
    char ch = buffer[(*consumer_index)++];
    *count -= 1;
    pthread_mutex_unlock(mutex); // unlocking the mutex to signal that the lineSeparator thread can process this data
    return ch;
} // end of "getBuffer1" function



void* readInputFromStdin(void* arg) {
    char* checkForStop = malloc(sizeof(char) * 5);
    while (true) {
        if (count_1 == BUFFER_SIZE) break; // breaking out of the main loop if the buffer size is met

        char ch; // initializing a char variable
        while (true) {
            ch = getchar();
            if (ch == '\n') { // when encounter a newline, the code in this if statement checks that the 'STOP' command did not come before it. This is done by getting the last 4 letters before the newline and checking if they say 'STOP'. If so, we are setting the stop condition to true and breaking out of the loop.
                strncpy(checkForStop, buffer_1 + producer_index_1 - 5, 4);
                checkForStop[4] = '\0';
                if (strcmp(checkForStop, "STOP") == 0) shouldStopInput = true;
                printf("STOP: |%s|\n", checkForStop);
                break;
            }
            if (ch == EOF) break; // only adding it to my buffer and looping if the char is not a newline or end of file
            putInBuffer(ch, buffer_1, &mutex_1, &full_1, &count_1, &producer_index_1); // calling helper function to properly place the char in the buffer.
        }
    }
    free(checkForStop); // freeing allocated memory
    return NULL; // returning a null pointer as the functions return value
} // end of "getInput" function


void* replaceLineSeparatorWithSpace(void* arg) {
    while (true) {
        if (shouldStopInput) break;
        char ch = getBuffer(buffer_1, &mutex_1, &full_1, &count_1, &consumer_index_1);
        if (ch == '\n') ch = ' ';
        putInBuffer(ch, buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2);
    }
    return NULL;
} // end of "replaceLineSeparatorWithSpace" function


void* replacePlusSignPairWithCaret(void* arg) {
    return NULL;
} // end of "replacePlusSignPairWithCaret" function


void* getOutput(void* arg) {
    return NULL;
} // end of "getOutput" function




int main(int argc, char* argv[]) {

    pthread_t input_t, lineSeparator_t, plusSign_t, output_t; // 'input', 'line separator', 'plus sign', and 'output' threads


    /* Creating the input threads including error handling */
    if (pthread_create(&input_t, NULL, readInputFromStdin, NULL) != 0) { // creating input_t
        fprintf(stderr, "Error creating input_t thread\n");
        return THREAD_CREATE_ERROR;
    }
    if (pthread_create(&lineSeparator_t, NULL, replaceLineSeparatorWithSpace, NULL) != 0) { // creating lineSeparator_t
        fprintf(stderr, "Error creating lineSeparator_t thread\n");
        return THREAD_CREATE_ERROR;
    }
    if (pthread_create(&plusSign_t, NULL, replacePlusSignPairWithCaret, NULL) != 0) { // creating plusSign_t
        fprintf(stderr, "Error creating plusSign_t thread\n");
        return THREAD_CREATE_ERROR;
    }
    if (pthread_create(&output_t, NULL, getOutput, NULL) != 0) { // creating output_t
        fprintf(stderr, "Error creating output_t thread\n");
        return THREAD_CREATE_ERROR;
    }


    /* Joining the threads including error handling */
    if (pthread_join(input_t, NULL) != 0) {                     // joining input_t
        fprintf(stderr, "Error joining input_t thread\n");
        return THREAD_JOIN_ERROR;
    }
    if (pthread_join(lineSeparator_t, NULL) != 0) {             // joining lineSeparator_t
        fprintf(stderr, "Error joining lineSeparator_t thread\n");
        return THREAD_JOIN_ERROR;
    }
    if (pthread_join(plusSign_t, NULL) != 0) {                  // joining plusSign_t
        fprintf(stderr, "Error joining plusSign_t thread\n");
        return THREAD_JOIN_ERROR;
    }
    if (pthread_join(output_t, NULL) != 0) {                    // joining output_t
        fprintf(stderr, "Error joining output_t thread\n");
        return THREAD_JOIN_ERROR;
    }

    return 0;
} // end of "main" function
