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

char charsOfInterest[] = "+";


bool isCharOfInterest(char ch) {
    int i = 0;
    while (i < strlen(charsOfInterest)) { // looping through the charsOfInterest to see if the given char is in it
        if (ch == charsOfInterest[i++]) return true;
    } // end of while loop
    return false;
} // end of "isCharOfInterest" function


bool containsStop(char* buffer, int producer_index, bool isNewlineReplacedBySpace) {
    char* checkForStop = malloc(sizeof(char) * 5); // creating a string for
    strncpy(checkForStop, buffer + producer_index - 4, 4); // getting a substring of the buffer, 4 chars back. I am checking to see if this string has the word "STOP"
    checkForStop[4] = '\0'; // appending the null terminator;
    if (strcmp(checkForStop, "STOP") == 0){ // checking if the buffer, 4 spots back, has the words STOP
        free(checkForStop); // free allocating memory
        if (isNewlineReplacedBySpace && buffer[producer_index - 5] == ' ') return true; // if the buffer has the word stop and the newline char has been replaced with a space, I am checking if there is a space 5 chars back to ensure that the STOP is isolated. If so, return true
        else if (!isNewlineReplacedBySpace && buffer[producer_index - 5] == '\n') return true; // similar to the command above expect checking for newline
    }
    return false; // if neither return true was triggered above, return false
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
    char ch = buffer[(*consumer_index)++]; // getting the char
    *count -= 1; // iterating the count
    pthread_mutex_unlock(mutex); // unlocking the mutex to signal that the lineSeparator thread can process this data
    return ch; // returning the char
} // end of "getBuffer1" function



void* readInputFromStdin(void* arg) {
    char* checkForStop = malloc(sizeof(char) * 5);
    bool breakOut = false;
    while (true) {
        char ch; // initializing a char variable
        while (true) {
            ch = getchar(); // getting the char from stdin;
            if (ch == EOF) {
                break; // only adding it to my buffer and looping if the char is not an end of file
            }
            if (ch == '\n') { // when encounter a newline, the code in this if statement checks that the 'STOP' command did not come before it. This is done by getting the last 4 letters before the newline and checking if they say 'STOP'. If so, we are setting the stop condition to true and breaking out of the loop.
                breakOut = containsStop(buffer_1, producer_index_1, false); // checking if the buffer contains the stop command. If it does, break out
                putInBuffer(ch, buffer_1, &mutex_1, &full_1, &count_1, &producer_index_1); // calling helper function to properly place the char in the buffer.
                if (breakOut) break;
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
        if (ch != '\n') putInBuffer(ch, buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2); // read the char into buffer 2 if it doesn't equal the newline
        else { // in the case that the char is a newline we have to handle it more carefully
            needsToStop = containsStop(buffer_2, producer_index_2, true); // we need to stop if the buffer contains the stop command. Calling upon helper function to check
            if (needsToStop) {
                putInBuffer('\n', buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2); // we keep the newline appended to the back on this. All other newlines are turned into spaces
                break; // breaking out
            }
            else putInBuffer(' ', buffer_2, &mutex_2, &full_2, &count_2, &producer_index_2); // appending a space in place of the newline; the program only makes it here if the current char is a newline and the STOP command is not present.
        }
    } // end of while loop
    return NULL;
} // end of "replaceLineSeparatorWithSpace" function


void* replacePlusSignPairWithCaret(void* arg) {
    char previousCh = ' ';
    while (true) {
        char ch = getBuffer(buffer_2, &mutex_2, &full_2, &count_2, &consumer_index_2); // getting the char from the buffer
        if (previousCh == '+' && ch == '+') { // if the previous char and the current char are both '+'
            ch = '^'; // set the current char to caret
            previousCh = ' '; // reset the previous char
            count_3 -= 1; // decrementing these values so that we override the last plus
            producer_index_3 -= 1;
            putInBuffer(ch, buffer_3, &mutex_3, &full_3, &count_3, &producer_index_3); // insert the caret into the buffer;
        }
        else { // if the last two chars were not pluses, then carry on as normal, placing the current ch in the buffer
            if (ch == '\n' && containsStop(buffer_3, producer_index_3, true)) { // if the stop command is break, break out
                putInBuffer(ch, buffer_3, &mutex_3, &full_3, &count_3, &producer_index_3); // insert char into buffer
                stop_index = producer_index_3 - 7; // once the stop command has made it to buffer three, we store this value to track it for the sake of the output
                break;
            }
            putInBuffer(ch, buffer_3, &mutex_3, &full_3, &count_3, &producer_index_3); // insert char into buffer
            previousCh = ch; // track the previous char for next iteration as the current char
        }
    } // end of while loop
    return NULL;
} // end of "replacePlusSignPairWithCaret" function


void* getOutput(void* arg) {
    int linesWritten = 0; // tracking the lines we have written
    int linesNeeded = 0; // tracking the lines we need to write
    while (true) {
        char ch;
        int i = 0; // tracking an index to ensure only 80 chars are sent
        linesNeeded = (stop_index != -1) ? (stop_index / NUM_ITEMS_PER_LINE) : 0; // getting the number of lines that we do need to write. This is based on when the stop index var is given a value other than it's initial (-1). Until then, we assume that we need to write forever.
        if (linesNeeded != 0 && linesWritten == linesNeeded) break;  // if the linesNeeded are not 0, meaning we have a stop index and the lines written equal the lines needed, meaning we have written all that is necessary, we break.

        while (i++ < NUM_ITEMS_PER_LINE) { // only printing 80 chars, plus the newline that is outside of this loop
            if (consumer_index_3 == stop_index) break; // if we encounter the stop_index, just break;
            ch = getBuffer(buffer_3, &mutex_3, &full_3, &count_3, &consumer_index_3); // get the char from the buffer
            printf("%c", ch); // print the char
        } // end of while loop (i++ < NUM_ITEMS_PER_LINE)

        printf("\n"); // printing the newline char
        linesWritten += 1;
    } // end of while loop (true)
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
