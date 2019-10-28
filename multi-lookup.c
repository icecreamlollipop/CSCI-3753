/* Include libraries:
- stdio.h: Allows IO operations
- stdlib.h: Allows exit()
- ctype.h: Allows isdigit()
- string.h: Allows strlen()
- pthread.h: Allows usage of pthreads
- unistd.h: Allows gettid()
- sys/syscall.h: Allows syscall()
- util.h: Allows dns_lookup() */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "util.h"

/* Define macros:
- gettid(): Allows gettid()
- MAX_NAME_LENGTH: Domain name length limit
- MAX_PRODUCE: Num producer threads limit
- MAX_CONSUMER: Num consumer threads limit
- MAX_DATA_FILES: Num data files limit
- MAX_ARGUMENTS: Num argc limit
- BUFFER_SIZE: Size of shared memory buffer */
#define gettid() syscall(SYS_gettid)
#define MAX_NAME_LENGTH 1025
#define MAX_PRODUCER 5
#define MAX_CONSUMER 10
#define MAX_DATA_FILES 10
#define MAX_ARGUMENTS 15
#define BUFFER_SIZE 5

/* Synchronization tools:
- condition variable
- mutex lock */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_con = PTHREAD_MUTEX_INITIALIZER;

/* README
- To compile: gcc multi-lookup.c util.c -o multi-lookup -pthread -Wall -Wextra
	- pthread: Allows usage of pthreads
- To run: valgrind ./multi-lookup <# requester> <# resolver> <requester log> <resolver log> <data file>...<data file>
	- valgrind: Checks for memory leaks
	- <# requester>: Num of producer threads
	- <# resolver>: Num of consumer threads
	- <requester log>: Write producer status info into this file
	- <resolver log>: Write consumer status info into this file
	- <data file>: Files that contain domain names
- Example: valgrind ./multi-lookup 1 1 serviced.txt results.txt names1.txt names2.txt names3.txt names4.txt names5.txt */













/* Helper functions

- usage()
	- Input: argv[0] and argc
	- Print ERROR and EXIT if not enough arguments

- isnumber()
	- Input: A string and its length
	- Return 0 if string is int; else, return 1

- get_num_producer()
	- Input: argv[1] and MAX_PRODUCER
	- Print ERROR and EXIT if argv[1] is not an int or exceeds max
	- Return <# requester>

- get_num_consumer()
	- Input: argv[2] and MAX_CONSUMER
	- Print ERROR and EXIT if argv[2] is not an int or exceeds max
	- Return <# resolver>

- open_producer_log()
	- argv[3] and file pointer
	- Print ERROR and EXIT if file path not valid
	- Opens <requester log> and returns file pointer

- open_consumer_log()
	- argv[4] and file pointer
	- Print ERROR and EXIT if file path not valid
	- Opens <resolver log> and returns file pointer

- get_num_data_files():
	- argc, MAX_ARGUMENTS, MAX_DATA_FILES
	- Print ERROR and EXIT if data files exceeds max
	- Return # of <data file>

- open_data_file()
	- argv[5] and file pointer
	- Print ERROR if file path not valid; returns NULL
	- Opens <data file> and returns file pointer

- get_num_domains()
	- file pointer
	- Returns num domains in given file */

void usage(char *str, int num){
	if(num < 6){
        printf("Usage: %s <# requester> <# resolver> <requester log> <resolver log> <data file>...<data file>\n", str);
        exit(1);	
	}
}

int isnumber(char *str, int size){
	for(int i = 0; i < size; i++){
		if(!isdigit(str[i])){
			return 1;
		}
	}
	return 0;
}

int get_num_producer(char *str){
    if(isnumber(str, strlen(str))){
    	printf("<# requester> must be an integer\n");
    	exit(1);
    }
    else if(atoi(str) > MAX_PRODUCER){
    	printf("<# requester> must not exceed %d\n", MAX_PRODUCER);
    	exit(1);
    }
    return atoi(str);
}

int get_num_consumer(char *str){
   	if(isnumber(str, strlen(str))){
     	printf("<# resolver> must be an integer\n");
    	exit(1);
    }
    else if(atoi(str) > MAX_CONSUMER){
    	printf("<# resolver> must not exceed %d\n", MAX_CONSUMER);
    	exit(1);
    }
	return atoi(str);
}

FILE *open_producer_log(char *str, FILE *fp){
   	if(!(fp = fopen(str, "r+"))){
   		printf("<requester log> file path is not valid\n");
   		exit(1);
   	}
   	return fp;
}

FILE *open_consumer_log(char *str, FILE *fp){
   	if(!(fp = fopen(str, "r+"))){
   		printf("<resolver log> file path is not valid\n");
   		exit(1);
   	}
   	return fp;
}

int get_num_data_files(int num){
   	if(num > MAX_ARGUMENTS){
   		printf("# of <data file> must not exceed %d\n", MAX_DATA_FILES);
   		exit(1);
   	}
   	return num - 5;
}


FILE *open_data_files(char *str, FILE *fp, int flag){
    if(!(fp = fopen(str, "r+")) && flag == 0){
    	printf("%s file path is not valid; moving on to next file\n", str);
    }
    return fp;
}

int get_num_domains(FILE *fp){
	int num_domains = 0;
  	char c;
  	for(c = getc(fp); c != EOF; c = getc(fp)){
  		if(c == '\n'){
  			num_domains++;
  		}
  	}
	return num_domains;
}









/* Parameter of thread functions
- num_data_files: The number of data files to be serviced, total
- num_data_files_done: The number of data files that have been serviced
- num_domains: The number of domain names
- buffer_idx: The index to put the element in buffer
- producer_idx: Producer buffer index
- consumer_idx: Consumer buffer index
- buffer: The shared memory buffer
- data_files: The data files */
struct param{
	int num_data_files;
  	int num_data_files_done;
  	int num_domains;
  	int num_domains_done;
  	int producer_idx;
  	int consumer_idx;
  	int counter;
  	char (*buffer)[MAX_NAME_LENGTH];
  	FILE **data_files;
  	FILE *producer_log;
  	FILE *consumer_log;
};












/* Consumer function
- Input: p, a structure of type struct param */

void *consume(void *arg){

	printf("tid = %ld\n", gettid());

	/* Cast the void parameter into a type of struct param */
  	struct param *p = (struct param*) arg;
  	char ip_address[INET6_ADDRSTRLEN];

  	/* Iterate through each domain name */
  	while(p->num_domains_done < p->num_domains){

  		printf("i = %d\n", i);

    	pthread_mutex_lock(&mutex);

		/* If the buffer is consumed empty, the consumer will wait() until the buffer is produced full or all files have been read */ 
	    while(p->producer_idx == 0){
    		pthread_cond_wait(&cond, &mutex);
    	}
    
    	/* When the buffer is full, consume all the domain names in the buffer
    	- Write domain name to results.txt
    	- Memset ip-address string
    	- Copy domain name to temp string and perform dns_lookup() */
	    fputs(p->buffer[p->consumer_idx], p->consumer_log);
	    fputs(",", p->consumer_log);
	    
	    memset(ip_address, 0, sizeof(ip_address));

		if(dnslookup(p->buffer[p->consumer_idx], ip_address, INET6_ADDRSTRLEN) == 0){
		   	fputs(ip_address, p->consumer_log);
		}
	    fputs("\n", p->consumer_log);

	    printf("p->consumer_idx = %d\n", p->consumer_idx);

    	
    	/* Update the consumer index of the buffer */
    	p->consumer_idx++;
    	p->num_domains_done++;
    	if(p->consumer_idx == BUFFER_SIZE){
    		p->consumer_idx = 0;
    		p->producer_idx = 0;
    	}
    
    	/* When the buffer has been emptied, call signal() on producer */
    	if(p->consumer_idx == 0){
		    pthread_cond_signal(&cond);
		}

    	pthread_mutex_unlock(&mutex);

  	}

  	printf("Exit success\n");

	return NULL;
}





/* Producer function
- Input: p, a structure of type struct param */

// TODO: Print gettid() and num_serviced to serviced.txt before exiting
void *produce(void *arg){


	/* Cast the void parameter into a type of struct param */
  	struct param *p = (struct param*) arg;

  	/* Initialize variables for reading the file */
  	char *line = NULL;
  	size_t n = 0;


  	/* Service each data file, if the data file is valid */
  	while(p->num_data_files_done < p->num_data_files){

    	pthread_mutex_lock(&mutex);
    	printf("tid = %ld\n", gettid());
    	

    	/* If the file is valid, iterate through each domain name in the data file */
    	printf("num data files done = %d\n", p->num_data_files_done);
    	if(p->num_data_files_done >= p->num_data_files){
    		pthread_cond_signal(&cond);
    		pthread_mutex_unlock(&mutex);
    		break;
    	}

    	if(p->data_files[p->num_data_files_done] != NULL){

    		while(getline(&line, &n, p->data_files[p->num_data_files_done]) != -1){
    	
    			/* If the buffer is produced full, the producer will wait() until buffer is consumed empty */
    			while(p->producer_idx == BUFFER_SIZE){
      				pthread_cond_wait(&cond, &mutex);
    			}

    			printf("p->producer_idx = %d\n", p->producer_idx);
      	
      			/* If the domain length is too long, write "DOMAIN NAME EXCEEDED MAX LENGTH" to the buffer */
		      	if(strlen(line) > MAX_NAME_LENGTH){
		      		strcpy(p->buffer[p->producer_idx], "DOMAIN NAME EXCEEDED MAX LENGTH");
      			}

      			/* When the buffer is empty, produce domain names to the buffer
      			- Get rid of the '\n' at the end of the line */
      			else{
      				fputs(line, p->producer_log);
	      			line[strlen(line) - 1] = 0;
    	  			strcpy(p->buffer[p->producer_idx], line);
      			}


	      		/* Update the producer index of the buffer */
	    	  	p->producer_idx++;
	    	  	p->counter++;
	    	  	printf("p->counter = %d\n", p->counter);

    	  		/* Call signal() on consumer when buffer is produced full, or all files have been read */
    	  		if(p->producer_idx == BUFFER_SIZE || p->counter == p->num_domains){
    	  			printf("\nsend signal p->producer_idx = %d\n", p->producer_idx);
    	  			printf("send signal p->counter = %d\n\n", p->counter);
	      			pthread_cond_signal(&cond);
	      		}

	      		printf("producer stuck 0\n");
    		}
		}

		/* Increment num data files serviced */
	    p->num_data_files_done++;
	    
	    pthread_mutex_unlock(&mutex);
  	}

	free(line);
	printf("producer exit\n");
	return NULL;
}



















int main(int argc, char **argv){





	/* Initialize variables
    - num_producer: The number of producer threads
    - num_consumer: The number of consumer threads
    - num_data_files: The number of data files
    - num_domains: The number of domains
    - producer_log: serviced.txt
    - consumer_log: results.txt
    - buffer: Shared memory buffer
    - p: Parameter for thread init functions */
	int num_producer = 0;
	int num_consumer = 0;
	int num_data_files = 0;
	int num_domains = 0;
 	FILE *producer_log = NULL;
	FILE *consumer_log = NULL;
  	char buffer[BUFFER_SIZE][MAX_NAME_LENGTH];
  	for(int i = 0; i < BUFFER_SIZE; i++){
   		memset(buffer[i], 0, sizeof(buffer[i]));
  	}
  	struct param p;







  	/* Read user arguments; helper functions check for error */
  	usage(argv[0], argc);
  	num_producer = get_num_producer(argv[1]);
  	num_consumer = get_num_consumer(argv[2]);
  	producer_log = open_producer_log(argv[3], producer_log);
  	consumer_log = open_consumer_log(argv[4], consumer_log);




  	
  	/* Get number of data files */
  	num_data_files = get_num_data_files(argc);
  	FILE **data_files = malloc(sizeof(FILE*) * num_data_files);
  	
  	/* Store all data files in a queue */
  	for(int i = 0; i < num_data_files; i++){

  		data_files[i] = open_data_files(argv[i + 5], data_files[i], 0);
    	
    	/* Count the number of domains in each data file */
    	if(data_files[i] != NULL){
    		num_domains += get_num_domains(data_files[i]);
    	}
  	}


  	/* Close all data files to reset getline() */
  	for(int i = 0; i < num_data_files; i++){
    	if(data_files[i] != NULL){
      		fclose(data_files[i]);
    	}
  	}

  	/* Re-open all data files */
  	for(int i = 0; i < num_data_files; i++){
    	data_files[i] = open_data_files(argv[i + 5], data_files[i], 1);
  	}
  




  	/* Initialize elements of type struct param */
  	p.num_data_files = num_data_files;
  	p.num_data_files_done = 0;
  	p.num_domains = num_domains;
  	p.num_domains_done = 0;
  	p.counter = 0;
  	p.producer_idx = 0;
  	p.consumer_idx = 0;
  	p.buffer = buffer;
  	p.data_files = data_files;
  	p.consumer_log = consumer_log;
  	p.producer_log = producer_log;











  	/* Create producer and consumer threads (who does dns lookup) */
  	pthread_t tids_producer[num_producer];
  	pthread_t tids_consumer[num_consumer];

  	
  	for(int i = 0; i < num_producer; i++){
    	pthread_create(&tids_producer[i], NULL, produce, &p);
  	}
  	for(int i = 0; i < num_consumer; i++){
    	pthread_create(&tids_consumer[i], NULL, consume, &p);
  	}

  	for(int i = 0; i < num_producer; i++){
    	pthread_join(tids_producer[i], NULL);
  	}
 	for(int i = 0; i < num_consumer; i++){
    	pthread_join(tids_consumer[i], NULL);
  	}
  	








  	/* Return gracefully
    - close all files that were opened
    - free all memory allocated */
  	fclose(producer_log);
  	fclose(consumer_log);
  	for(int i = 0; i < num_data_files; i++){
    	if(data_files[i] != NULL){
      		fclose(data_files[i]);
    	}
  	}
  	

  	free(data_files);

	return 0;
}