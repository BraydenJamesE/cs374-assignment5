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

#define BUFFER_SIZE 50050
#define NUM_ITEMS_PER_LINE 80 // this value is the number of items that should be outputted per line. It is less than the buffer size, allowing us to model the buffer as being unbounded.
#define LOOP_LIMIT_SAFETY_VAL_ONE 400
#define LOOP_LIMIT_SAFETY_VAL_TWO 400


/* Buffer 1 Globals */
char buffer_1[BUFFER_SIZE];
int count_1 = 0; // number of items in the buffer
int producer_index_1 = 0; // index where the input thread will put the next item
int consumer_index_1 = 0; // index where the linear separator thread will put the next item
int stop_index = -1;
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


bool outputComplete = false;
bool inputComplete = false;
char charsOfInterest[] = "+";


bool isCharOfInterest(char ch) {
    int i = 0;
    while (i < strlen(charsOfInterest)) { // looping through the charsOfInterest to see if the given char is in it
        if (ch == charsOfInterest[i++]) return true;
    } // end of while loop
    return false;
} // end of "isCharOfInterest" function


bool containsStop(char* buffer, int producer_index, bool isNewlineReplacedBySpace) {
    char* checkForStop = malloc(sizeof(char) * 5);
    char* lastFew = malloc(sizeof(char) * 15);
    size_t sizeofLastFew = sizeof(lastFew);
    memset(lastFew, '\0', sizeofLastFew);
    strncpy(checkForStop, buffer + producer_index - 4, 4);
    strncpy(lastFew, buffer + producer_index - 11, 10);
    checkForStop[4] = '\0';
    fprintf(stderr, "Last Few: %s\n", lastFew);
    fprintf(stderr, "CheckforStopString: %s\n", checkForStop);
    fprintf(stderr, "buffer[producer_index - 5] == '\n': |%d| and buffer[producer_index - 5] = |%c|\n", buffer[producer_index - 5] == '\n', buffer[producer_index - 5]);
    if (strcmp(checkForStop, "STOP") == 0){
        if (isNewlineReplacedBySpace && buffer[producer_index - 5] == ' ') {
            fprintf(stderr, "RETURNING TRUE (CONTAINS SPACE).\n");
            return true;
        }
        else if (!isNewlineReplacedBySpace && buffer[producer_index - 5] == '\n') {
            fprintf(stderr, "RETURNING TRUE (CONTAINS newline).\n");
            return true;
        }
    }
    fprintf(stderr, "RETURNING FALSE.\n");
    return false;
} // end of "containsStop" function


void putInBuffer(char item, char* buffer, pthread_mutex_t* mutex, pthread_cond_t* full_t, int* count, int* producer_index) {
    pthread_mutex_lock(mutex); // locking the mutex before writing to the buffer
    buffer[(*producer_index)++] = item; // adding the char to the buffer and iterating producer_index
    *count += 1; // iterating the amount of items in buffer
    if (!isCharOfInterest(item)) pthread_cond_signal(full_t); // telling the consumer that the buffer is no longer empty if the value is not a char of interest. If it is, we hold off to see if the buffer will maintain that value.
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
    int safetyIndexToAvoidLongLoopsWhileTesting = 0;
    int safetyIndexToAvoidLongLoopsWhileTestingTwo = 0;

    char* checkForStop = malloc(sizeof(char) * 5);
    bool breakOut = false;
    while (true) {
        char ch; // initializing a char variable
        while (true) {
            ch = getchar();
            if (ch == EOF) {
                break; // only adding it to my buffer and looping if the char is not a  end of file
            }
            if (ch == '\n') { // when encounter a newline, the code in this if statement checks that the 'STOP' command did not come before it. This is done by getting the last 4 letters before the newline and checking if they say 'STOP'. If so, we are setting the stop condition to true and breaking out of the loop.
                breakOut = containsStop(buffer_1, producer_index_1, false);
                putInBuffer(ch, buffer_1, &mutex_1, &full_1, &count_1, &producer_index_1); // calling helper function to properly place the char in the buffer.
                if (breakOut) {
                    fprintf(stderr, "BREAKING FROM readInputFromStdin\n");
                    break;
                }
            }
            else {
                putInBuffer(ch, buffer_1, &mutex_1, &full_1, &count_1, &producer_index_1); // calling helper function to properly place the char in the buffer.
            }
        }
        if (breakOut) break;
    }
    free(checkForStop); // freeing allocated memory
    return NULL; // returning a null pointer as the functions return value
} // end of "getInput" function

void* replaceLineSeparatorWithSpace(void* arg) {
    while (true) {
        char ch = getBuffer(buffer_1, &mutex_1, &full_1, &count_1, &consumer_index_1); // getting the char from the buffer
        bool needsToStop = false;
        if (ch != '\n') putInBuffer(ch, buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2); // read the char into buffer 2
        else { // in the case that the char is a newline we have to handle it more carefully
            fprintf(stderr, "NEWLINE FOUND\n");
            needsToStop = containsStop(buffer_2, producer_index_2, true);
            if (needsToStop) {
                putInBuffer('\n', buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2); // read the char into buffer 2
                fprintf(stderr, "BREAKING FROM replaceLineSeparatorWithSpace\n");
                break;
            }
            else putInBuffer(' ', buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2); // read the char into buffer 2
        }
    }
    return NULL;
} // end of "replaceLineSeparatorWithSpace" function


void* replacePlusSignPairWithCaret(void* arg) {
    int safetyIndexToAvoidLongLoopsWhileTesting = 0;
    char previousCh = '\0';
    while (true) {
        char ch = getBuffer(buffer_2, &mutex_2, &full_2, &count_2, &consumer_index_2);
        if (previousCh == '+' && ch == '+') { // if the previous char and the current char are both '+'
            ch = '^'; // set the current char to caret
            previousCh = '\0'; // reset the previous char
            count_3 -= 1;
            producer_index_3 -= 1;
            putInBuffer(ch, buffer_3, &mutex_3, &full_3, &count_3, &producer_index_3); // insert the caret into the buffer;
        }
        else { // if the last two chars were not pluses, then carry on as normal, placing the current ch in the buffer
            if (ch == '\n' && containsStop(buffer_3, producer_index_3, true)) {
                putInBuffer(ch, buffer_3, &mutex_3, &full_3, &count_3, &producer_index_3); // insert char into buffer
                fprintf(stderr, "BREAKING FROM replacePlusSignPairWithCaret\n");
                stop_index = producer_index_3 - 7;
                fprintf(stderr, "stop_index = |%d|  and    buffer[stop_index] = |%c|\n", stop_index, buffer_3[stop_index]);
                break;
            }
            putInBuffer(ch, buffer_3, &mutex_3, &full_3, &count_3, &producer_index_3); // insert char into buffer
            previousCh = ch; // track the previous char for next iteration as the current char
        }
    } // end of while loop
    return NULL;
} // end of "replacePlusSignPairWithCaret" function


void* getOutput(void* arg) {
    int safetyIndexToAvoidLongLoopsWhileTesting = 0;
    int linesWritten = 0;
    while (true) {
        char ch;
        int i = 0; // tracking an index to ensure only 80 chars are sent
        fprintf(stderr, "linesWritten: %d\n", linesWritten);
        if (stop_index != -1 && stop_index / NUM_ITEMS_PER_LINE == linesWritten) outputComplete = true;
        if (outputComplete) {
            printf("\n");
            fprintf(stderr, "BREAKING FROM getOutput initial outComplete\n");
            break;
        }
        while (i++ < NUM_ITEMS_PER_LINE) { // only printing 80 chars, plus the newline that is outside of this loop
            if (consumer_index_3 == stop_index){
                outputComplete = true;
                fprintf(stderr, "BREAKING FROM getOutput (consumer_index_3 is stop index - 2)\n");
                break;
            }
            ch = getBuffer(buffer_3, &mutex_3, &full_3, &count_3, &consumer_index_3);
            printf("%c", ch);
        } // end of while loop
        printf("\n"); // printing the newline char
        linesWritten += 1;
    }
    fprintf(stderr, "BREAKING FROM getOutput (end of function) \n");
    return NULL;
} // end of "getOutput" function


void printVarsForTesting() { // creating a function to print the buffers for testing purposes.
    buffer_1[strlen(buffer_1)] = '\0';
    buffer_2[strlen(buffer_2)] = '\0';
    buffer_3[strlen(buffer_3)] = '\0';
    fprintf(stderr, "buffer_1:\n %s\n\n", buffer_1);
    fprintf(stderr, "buffer_2:\n %s\n\n", buffer_2);
    fprintf(stderr, "buffer_3:\n %s\n\n", buffer_3);
} // end of "printVarsForTesting"



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
