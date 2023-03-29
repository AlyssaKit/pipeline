#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define SIZE 1000
#define NUM_ITEMS 50

// set up first buffer 
int count = 0;
char* buffer_1[SIZE];
int count_1 = 0;
int prod_idx_1 = 0;
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

// set up second buffer for putting the results of
// the line seperator thread into
char* buffer_2[SIZE];
int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// set up third buffer for putting the results of
// the change plus sign thread into 
char* buffer_3[SIZE];
int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


void put_buff_1(char* item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    buffer_1[prod_idx_1] = item;
    // Increment the index where the next item will be put.
    prod_idx_1 = (prod_idx_1 + 1) % SIZE;
    count_1++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
}


void *get_input(void *args) {
    int i = 0;
    char item[1000];
    while (i < NUM_ITEMS) {
        // read in from user or file, if it doesn't read anything in it will break due
        // to the error
        if (fgets(item, 1000, stdin) == NULL) {
            break;
        }
        // calloc memory for the read in string so that it can be placed in the buffer
        // and changed in later functions
        char *myLine = (char*)calloc(strlen(item)+1, sizeof(char));
        if (myLine == NULL) {
            printf("Memory not allocated.\n");
            exit(0);
        }
        // put the read in string into the heap memory pointer myLine
        strcpy(myLine, item);
        // erase all the stuff that was in item that isn't needed for next time
        memset(item, '\0', sizeof(item));
        
        if (strncmp(myLine, "STOP\n", 5) == 0) {
            put_buff_1(myLine); // put the "STOP\n" into the buffer to signal the other threads to stop
            break; // break the loop when "STOP\n" is read
        }
        // puts the read in string into the first buffer
        put_buff_1(myLine);
        i = i + 1;
    }
    return NULL;
}

/*
Get the next item from buffer 1
*/
char* get_buff_1(){
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);
    while (count_1 == 0){
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
    }
    char* item = buffer_1[con_idx_1];
    // Increment the index from which the item will be picked up
    con_idx_1 = (con_idx_1 + 1) % SIZE;
    count_1--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
    // Return the item
    return item;
}

/*
 Put an item in buff_2
*/
void put_buff_2(char* item){
    // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_2);
    // Put the item in the buffer
  buffer_2[prod_idx_2] = item;
    // Increment the index where the next item will be put.
  prod_idx_2 = (prod_idx_2 + 1) % SIZE;
  count_2++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_2);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
}

/*
 function that replaces all the newline characters with a space and puts the result into put_buff_2
*/
void *get_line_sep(void *args)
{
    char* item;
    for (int i = 0; i < NUM_ITEMS; i++)
    {
      // get string from buffer 1
      item = get_buff_1();
      // if it is STOP\n don't do anything, this helps for catching the 
      // correct stop in the output thread
      // if it is not STOP\n then replace the newline character with a space
      if(strncmp(item, "STOP\n", 5) != 0){
        item[strcspn(item, "\n")] = ' ';
      }
      // put the resulting item into buffer 2
      put_buff_2(item);
    }
    return NULL;
}


char* get_buff_2(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_2);
  while (count_2 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_2, &mutex_2);
  char* item = buffer_2[con_idx_2];
  // Increment the index from which the item will be picked up
  con_idx_2 = con_idx_2 + 1;
  count_2--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
  // Return the item
  return item;
}

/*
 Put an item in buff_3
*/

void put_buff_3(char* item){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_3);
    // Put the item in the buffer
    buffer_3[prod_idx_3] = item;
    // Increment the index where the next item will be put.
    prod_idx_3 = (prod_idx_3 + 1) % SIZE;
    count_3++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_3);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
}


/* function that replaces all the "++" with "^" */
void *get_plus_sign(void *args){
  // set up two strings to allow for removal of "++" and additon of "^"
  char lineOfWords[1000];
  char lineOfWordsCopy[1000];
  // initialize to empty
  memset(lineOfWords, '\0', sizeof(lineOfWords));
  memset(lineOfWordsCopy, '\0', sizeof(lineOfWordsCopy));
  int j;
  int a = 0;
  char* line;
  for (int i = 0; i < NUM_ITEMS; i++){
    // set up new heap memory for resulting string so that it can be used in put and get buff
    char *lineOfmyWords = (char*)calloc(1001, sizeof(char));
    if (lineOfmyWords == NULL) {
      printf("Memory not allocated.\n");
      exit(0);
    }
    // get string from buffer 2
    line = get_buff_2();
    // while character is not NULL
    while(line[a] != '\0'){
      // if this and the next characters are plus signs
      // then get previous string and add ^ to it, 
      // then add two to a to skip over the next plus sign
      if(line[a] == '+' && line[a+1] == '+'){
        j = snprintf(lineOfWordsCopy, 1000, "%s^", lineOfWords);
        j = snprintf(lineOfWords, 1000, "%s", lineOfWordsCopy);
        a = a + 2;        
        // else just put the char from line[a] into the previous string
      }else{
        j = snprintf(lineOfWordsCopy, 1000, "%s%c", lineOfWords, line[a]);
        j = snprintf(lineOfWords, 1000, "%s", lineOfWordsCopy);
        a = a + 1;        
      }
    }
    a = 0;
    // put the new string into lineOfmyWords which is the heap memory
    strcpy(lineOfmyWords, lineOfWords);
    // put that string into buffer 3 for output
    put_buff_3(lineOfmyWords);
    memset(lineOfWords, '\0', sizeof(lineOfWords));
    memset(lineOfWordsCopy, '\0', sizeof(lineOfWordsCopy));
  }
    return NULL;
}

/*
Get the next item from buffer 3
*/
char* get_buff_3(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_3);
  while (count_3 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_3, &mutex_3);
  char* item = buffer_3[con_idx_3];
  // Increment the index from which the item will be picked up
  con_idx_3 = con_idx_3 + 1;
  count_3--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
  // Return the item
  return item;
}



/*
 Function that the output thread will run. 
 consume item from buffer 3 and then when characters read in is equal to 80, then
 Print the 80 characters.
*/

void *write_output(void *args){
  char lineOfWords1[1000];
  char lineOfWordsCopy1[1000];
  int a = 0;
  int j;
  memset(lineOfWords1, '\0', sizeof(lineOfWords1));
  memset(lineOfWordsCopy1, '\0', sizeof(lineOfWordsCopy1));
  
  // just keep going until exit
  while(1){
    char* line = get_buff_3();
    // read in from buffer 3
    // check that character is not equal to NULL
    while(line[a] != '\0'){
      count = count + 1;
      // put character into the string that will be printed once count reaches
      // 80 
      j = snprintf(lineOfWordsCopy1, 1000, "%s%c", lineOfWords1, line[a]);
      j = snprintf(lineOfWords1, 1000, "%s", lineOfWordsCopy1);
      a = a + 1;   
      // if string is equal to the correct stop sign then it exits with exit success
      if(strncmp(line, "STOP\n", 5) == 0){
        exit(EXIT_SUCCESS);
      } 
      // once it gets enough characters it prints the correct string
      // and the resets count to 0 and empties out both strings used for 
      // getting the 80 char string
      if(count == 80){
        printf("%s\n", lineOfWords1);
        fflush(stdout);
        memset(lineOfWords1, '\0', sizeof(lineOfWords1));
        memset(lineOfWordsCopy1, '\0', sizeof(lineOfWordsCopy1));
        count = 0;
      } 
    }
    // empty out line
    memset(line, '\0', 1000);
    // restart a at 0 for the next line
    a = 0;
  }
  memset(lineOfWords1, '\0', sizeof(lineOfWords1));
  memset(lineOfWordsCopy1, '\0', sizeof(lineOfWordsCopy1));
  return NULL;
}


/*
Thread 1, called the Input Thread, reads in lines of characters from the standard input.
Thread 2, called the Line Separator Thread, replaces every line separator in the input by a space.
Thread 3, called the Plus Sign thread, replaces every pair of plus signs, i.e., "++", by a "^".
Thread 4, called the Output Thread, write this processed data to standard output as lines of exactly 80 characters.
*/


int main()
{   
    srand(time(0));
    memset(buffer_1, '\0', sizeof(buffer_1));
    memset(buffer_2, '\0', sizeof(buffer_2));
    memset(buffer_3, '\0', sizeof(buffer_3));
    pthread_t input_t, line_sep_t,plus_sign_t, output_t;
    // Create the threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_sep_t, NULL, get_line_sep, NULL);
    pthread_create(&plus_sign_t, NULL, get_plus_sign, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);
    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(line_sep_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);
    memset(buffer_1, '\0', sizeof(buffer_1));
    memset(buffer_2, '\0', sizeof(buffer_2));
    memset(buffer_3, '\0', sizeof(buffer_3));
    return EXIT_SUCCESS;
}