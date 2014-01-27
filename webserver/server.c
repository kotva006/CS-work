/* csci4061 F2013 Assignment 4 
* section: 4 
* date: 12/01/13 
* names: Benjamin, Ryan */

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>

#include "util.h"

#define MAX_THREADS 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_LENGTH 64
#define CACHE_MAX 100

pthread_mutex_t queue_mutex =    PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t count_mutex =    PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t queue_full =      PTHREAD_COND_INITIALIZER;

int s_exit;
int len_of_q = -1;
int t_count = 0;
int max_len;
int maxCacheCount;
int cacheSize = 0; 
int cachePos = 0;
int cacheCount;
double elap = 0.0;

double comp_time(struct timeval time_s, struct timeval time_e) {
  double elap = 0.0;

  if (time_e.tv_sec > time_s.tv_sec) {
    elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
    elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
  }
  else {
    elap = time_e.tv_usec - time_s.tv_usec;
  }
  return elap;
}
struct timeval time_s, time_e;

char *root;
FILE *log_file;

/*************************************************************
 *
 * The followin regex is going to be used to determine the
 * type of request.
 *
 */

regex_t html;
regex_t gif;
regex_t jpeg;
regex_t txt;

void init_regex() {
  if (regcomp(&html, "[.]html$", REG_EXTENDED)  != 0 ||
      regcomp(&gif,  "[.]gif$",  REG_EXTENDED)  != 0 ||
      regcomp(&jpeg, "[.]jpg$",  REG_EXTENDED)  != 0 ||
      regcomp(&txt,  "[.]txt$",  REG_EXTENDED)  != 0 ) {
        fprintf(stderr, "REGEX COMPILATION ERROR,\n Exiting...\n");
        exit(1);
    }
  return;
}

char *find_regex(char *in) {

  int check = regexec(&html, in, 0, NULL, 0);
  if (check != REG_NOMATCH) {return "html";}

  check = regexec(&gif, in, 0, NULL, 0);
  if (check != REG_NOMATCH) {return "gif";}
  
  check = regexec(&jpeg, in, 0, NULL, 0);
  if (check != REG_NOMATCH) {return "jpg";}

  check = regexec(&txt, in, 0, NULL, 0);
  if (check != REG_NOMATCH) {return "text";}
  return "text";
}
//*****  END REGEX **************************************


//Structure for queue.
typedef struct request_queue
{
	int		m_socket;
	char	m_szRequest[MAX_REQUEST_LENGTH];
} request_queue_t;

struct buffer{
   char location[MAX_REQUEST_LENGTH];
	 char buf[100000];
	 long int size;
};

//Stores the cache
struct buffer cache[CACHE_MAX];

pthread_t dispatcher[MAX_THREADS], workers[MAX_THREADS];
request_queue_t *queue;


//USED to try and cleanly exit 
void setexit() {
  s_exit = 1;
}

/*
 * This is the dispatcher function, if currently gets a request,
 * and adds it to the stack.
 */
void * dispatch(void * arg)
{

  while (1) {

    int fd = accept_connection();
    if (fd < 0)
      pthread_exit(NULL);
    char buf[1024];
    if (get_request(fd, buf) == 0) {
      if(pthread_mutex_lock(&queue_mutex) != 0)
        perror("Error locking queue mutex\n");

      while (len_of_q >= max_len) { //wait if stack is full
        pthread_cond_wait(&queue_full,&queue_mutex);
      }

      //add request to stack
      len_of_q++;
      queue[len_of_q].m_socket = fd;
      strcpy(queue[len_of_q].m_szRequest, buf);

      if(pthread_mutex_unlock(&queue_mutex) != 0)
        perror("Error unlocking queue mutex\n");
    }

    usleep(10000);
  }
	return NULL;
}


/*
 * This is the worker code it reads from the queue and process the request
 * then calls return_result() to return result
 */

void * worker(void * arg)
{
  // First set the number that this thread is
  if(pthread_mutex_lock(&count_mutex) != 0){
    perror("Error locking count mutex\n");
  }
  int thread_id = t_count;
  t_count++;
  int request_count = 0;
  if(pthread_mutex_unlock(&count_mutex) != 0){
    perror("Error unlocking count mutex\n");
  }

  // Start waiting for requests on the queue
  while(1) {
    while(0 != pthread_mutex_lock(&queue_mutex)){
      perror("Error locking queue mutex");
    }
    gettimeofday(&time_s,NULL);//Start timing
    if (len_of_q >= 0) {
      request_queue_t work = queue[len_of_q];
      log_file = fopen("server.log","a");
      if(log_file == NULL){
        perror("Error opening log file\n");
      }

      int cacheCount;
      for(cacheCount = 0; cacheCount < maxCacheCount; cacheCount++){
        if(strcmp(cache[cacheCount].location, work.m_szRequest) == 0){
          //Stop timer at return_result (CACHE entry)
          gettimeofday(&time_e,NULL);

          return_result(work.m_socket, find_regex(work.m_szRequest), 
                                 cache[cacheCount].buf, cache[cacheCount].size);

          fprintf(log_file, "[%i][%i][%i][%s][%li][%.3fms][HIT]\n", thread_id, 
                            request_count, work.m_socket, work.m_szRequest, 
                     cache[cacheCount].size, comp_time(time_s,time_e)/1000.0);

          if(fclose(log_file) != 0){
            perror("Error closing log file\n");
          }
          request_count++; len_of_q--;
				  cacheCount = -1;
          break; // break out of the for loop
	      }
      }

      //Catching a cache hit
      if (cacheCount == -1) {
        pthread_cond_signal(&queue_full);
        pthread_mutex_unlock(&queue_mutex); 
        continue; // restart the while loop
      }

      //get the location of the file to open
      char *file_loc=malloc(sizeof(char)*(strlen(work.m_szRequest)+strlen(root)));
      if (file_loc == NULL) {

        fprintf(log_file, "[%i][%i][%i][%s][%s]\n", thread_id, request_count,
                          work.m_socket, work.m_szRequest, "Error mallocing space");
        if(fclose(log_file) != 0){
          perror("Error closing log file\n");
        }
        return_error(work.m_socket,"Internal Error");
        pthread_cond_signal(&queue_full);
        pthread_mutex_unlock(&queue_mutex);
        request_count++; len_of_q--; 
        continue;
      }
      strcpy(file_loc,root);
      strcat(file_loc,work.m_szRequest);

      FILE *file = fopen( file_loc, "rb");
      if (file == NULL) {
        fprintf(log_file, "[%i][%i][%i][%s][%s]\n", thread_id, request_count, 
                         work.m_socket, work.m_szRequest, "FILE not found.");
        fclose(log_file);
        return_error(work.m_socket,"FILE not found.");
        free(file_loc);
        pthread_cond_signal(&queue_full);
        pthread_mutex_unlock(&queue_mutex); 
        request_count++; len_of_q--;
        continue;
      }
      //Find the size of the file and read it all into a buffer
      if(fseek(file,0,SEEK_END) != 0){
        perror("Error finding length of the file");
        fprintf(log_file, "[%i][%i][%i][%s][%s]\n", thread_id, request_count, 
                         work.m_socket, work.m_szRequest, "FILE read error.");
        fclose(log_file);
        return_error(work.m_socket,"Internal Error");
        pthread_cond_signal(&queue_full);
        pthread_mutex_unlock(&queue_mutex);
        request_count++; len_of_q--; 
        continue;
      }

      //reset the file pointer and read it in
      long size = ftell(file);
      char *buf = malloc(size + 2);
      if(fseek(file,0,SEEK_SET) != 0 || fread(buf,size,1,file) < 0){
        perror("Error setting the pointer to the start of the file\n");
        fprintf(log_file, "[%i][%i][%i][%s][%s]\n", thread_id, request_count, 
                         work.m_socket, work.m_szRequest, "FILE read error.");
        fclose(log_file);
        return_error(work.m_socket,"Internal Error");
        pthread_cond_signal(&queue_full);
        pthread_mutex_unlock(&queue_mutex);
        request_count++; len_of_q--; 
        continue;
      }

      //Stop timer at return_result (Disk entry)
      gettimeofday(&time_e,NULL);
      return_result(work.m_socket, find_regex(file_loc), buf, size);
      strcpy(cache[cachePos].location,work.m_szRequest);
			memcpy(cache[cachePos].buf,buf, size);
			cache[cachePos].size = size;
      //Checks that our cacheSize isn't too big
      if (cacheSize >= maxCacheCount)
        cacheSize = maxCacheCount;
      else
        cacheSize++;
      //If at end of cache, start overwriting old entries
      if (cachePos >= maxCacheCount - 1)
        cachePos = 0;
      else 
        cachePos++;

      //write to log
      fprintf(log_file, "[%i][%i][%i][%s][%li][%.3fms][MISS]\n", thread_id, request_count, 
                        work.m_socket, work.m_szRequest, size, 
                        comp_time(time_s,time_e) / 1000.0);      
      if(fclose(log_file) != 0){
        perror("Error closing log file\n");
      }
     
      //cleanup and unlock  
      free(buf);
      free(file_loc);
      if(fclose(file) != 0){
        perror("Error closing file\n");
      }

      len_of_q--;
      request_count++;
      if(pthread_cond_signal(&queue_full) != 0)
        perror("Error signaling queue_full\n");
			
    } //end len_of_q > 0 

    if(pthread_mutex_unlock(&queue_mutex) != 0)
      perror("Error unlocking queue mutex\n");
   
    usleep(10000);
  }
  return NULL;
}

int main(int argc, char **argv){
  //Error check first.
  if(argc != 7){
    printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n" , argv[0]);
    return -1;
  } else if(atoi(argv[6]) < 0 || atoi(argv[6]) > 100){
    printf("Invalid cache size. Please input cache size between 0 and 100 entries\n");
    return -1;
  }
  s_exit = 0; //initalize the state of the process
  
  //capture the inputs into variables
  int port = atoi(argv[1]);

  root = malloc(sizeof(char) * (strlen(argv[2]) + 1));
  if (root == NULL) {
    fprintf(stderr, "Error allocating root");
    exit(1);
  }
  strcpy(root, argv[2]); 

  int num_dispatch = atoi(argv[3]);
  int num_workers  = atoi(argv[4]);
  max_len = atoi(argv[5]);
  maxCacheCount = atoi(argv[6]);

  queue = (request_queue_t *) malloc(max_len * sizeof(request_queue_t));
  if (queue == NULL) {
    perror("Error allocating queue");
    free(root);
    exit(1);
  }

  struct sigaction act;//going to be used to safely exit server

  act.sa_handler = setexit;
  act.sa_flags   = 0;
  if (sigemptyset(&act.sa_mask) == -1 || sigaction(SIGINT,&act,NULL) != 0)
    fprintf(stderr, "Error setting signal mask!\n");
  else
    fprintf(stdout, "Press ^C to halt the server.\n");
  log_file = fopen("server.log", "w");
  fclose(log_file);
  init_regex();
  init(port);

  //Create the dispatch threads and the worker threads
  int i;
  int stat;
  for( i=0; i < num_dispatch; i++) {
    if(stat = pthread_create(&dispatcher[i], NULL, dispatch, NULL) != 0 || pthread_detach(dispatcher[i]) != 0) {
      fprintf(stderr, "Creating a dispatch thread %i, returned %i!\n", i, stat);
    }
  }

  for( i=0; i < num_workers; i++) {
    if(stat =  pthread_create(&workers[i], NULL, worker, NULL) != 0 || pthread_detach(workers[i]) != 0) {
      fprintf(stderr, "Creating a worker thread %i, returned %i!\n", i, stat);
    }
  }
  while(s_exit == 0){sleep(1);} //Used to shutdown the server

  // Start cleanup
  fprintf(stderr,"Exiting...\n");
  free(queue);
  free(root);
  return 0;
}
