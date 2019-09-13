#include "multi_lookup.h"
#include "util.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>

#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTER_THREADS 10
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define MAX_BUFFER_SIZE 50

void request(struct thread_args *args) {
	if (args->input_file_names[args->input_file_index] != NULL) {
		args->num_files_serviced++;

		// service file by adding hostnames to queue
		FILE *input_file = fopen(args->input_file_names[args->input_file_index], "r");
		if(input_file) {
			char line[MAX_NAME_LENGTH];
			while (fgets(line, MAX_NAME_LENGTH, input_file) != NULL) {
				//remove new line
				line[strlen(line) - 1] = '\0';
				// wait for space
				sem_wait(args->space_avail);
				// lock and add
				pthread_mutex_lock(args->mutex);
				if (*args->buf_index != MAX_BUFFER_SIZE - 1) {
					*args->buf_index = *args->buf_index + 1;
					args->buffer[*args->buf_index] = strdup(line);
					*args->num_added = *args->num_added + 1;
				}
				pthread_mutex_unlock(args->mutex);
				// signal items available
				sem_post(args->items_avail);
			}
			fclose(input_file);
		}
		else {
			printf("Failed to open %s.\n", args->input_file_names[args->input_file_index]);
		}

		// if there are more files to be serviced then call again else print files serviced
		if (args->input_file_index + args->num_requester_threads < MAX_INPUT_FILES) {
			args->input_file_index += args->num_requester_threads;
			request(args);
		}
		else {
			pthread_mutex_lock(args->service_file_mutex);
			fprintf(args->service_file, "Thread %d serviced %d files.\n", args->id, args->num_files_serviced);
 			pthread_mutex_unlock(args->service_file_mutex);
 		}
	}
	else {
		pthread_mutex_lock(args->service_file_mutex);
		fprintf(args->service_file, "Thread %d serviced %d files.\n", args->id, args->num_files_serviced);
		pthread_mutex_unlock(args->service_file_mutex);
	}
}

void resolve(struct thread_args *args) {
	// lock and check that there are still items and wait if not
	pthread_mutex_lock(args->mutex);
	if (*args->num_added != *args->num_removed) {
		pthread_mutex_unlock(args->mutex);
		sem_wait(args->items_avail);
	}
	else {
		pthread_mutex_unlock(args->mutex);
	}

	// lock and remove (dns)
	pthread_mutex_lock(args->mutex);
	while (*args->buf_index != -1){
		char hostname[20];
		dnslookup(args->buffer[*args->buf_index], hostname, 20);
		pthread_mutex_lock(args->results_file_mutex);
		fprintf(args->results_file, "%s,%s\n", args->buffer[*args->buf_index], hostname);
		pthread_mutex_unlock(args->results_file_mutex);

		free(args->buffer[*args->buf_index]);
		*args->buf_index = *args->buf_index - 1;
		*args->num_removed = *args->num_removed + 1;
		pthread_mutex_unlock(args->mutex);
		pthread_mutex_lock(args->mutex);
	}
	pthread_mutex_unlock(args->mutex);

	// signal space
	sem_post(args->space_avail);

	// lock and check if requesters are still there in case it adds more
	pthread_mutex_lock(args->mutex);
	if (*args->req_exists) {
		pthread_mutex_unlock(args->mutex);
		resolve(args);
	}
	else {
		pthread_mutex_unlock(args->mutex);
	}
}

int main(int argc, char *argv[]) {
	// timer
	struct timeval s;
	gettimeofday(&s, NULL);

	// ensure enough arguments
	if (argc < 5) {
		printf("Not enough arguments.\n");
		return -1;
	}

	//initiallizing variables
	char **array = malloc(MAX_BUFFER_SIZE * sizeof(char*));
	int array_index = -1;
	int a;
	for (a = 0; a < MAX_BUFFER_SIZE; a++) {
		array[a] = "";
	}

	bool req_exists = true;
	int removed = 0;
	int added = 0;
	
	pthread_mutex_t service_file_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t results_file_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	sem_t space_avail;
	sem_t items_avail;

	sem_init(&space_avail, 1, MAX_BUFFER_SIZE);
	sem_init(&items_avail, 1, 0);

	//get arguments
	int num_request = atoi(argv[1]);
	if (num_request > MAX_REQUESTER_THREADS) {
		printf("Too many requester threads. Only creating 5.\n");
		num_request = MAX_REQUESTER_THREADS;
	}

	int num_resolve = atoi(argv[2]);
	if (num_resolve > MAX_RESOLVER_THREADS) {
		printf("Too many resolver threads. Only creating 10.\n");
		num_resolve = MAX_RESOLVER_THREADS;
	}

	FILE *requester_log = fopen(argv[3], "w");
	if (!requester_log) {
		printf("Requester file failed to open.\n");
		return -1;
	}

	FILE *resolver_log = fopen(argv[4], "w");
	if (!resolver_log) {
		printf("Resolver file failed to open.\n");
		return -1;
	}

	char **input_files = malloc(MAX_INPUT_FILES * sizeof(char*));

	int i, num_input_files;
	int array_idx = 0;
	if (argc > 15) {
		printf("Too many input files. Only servicing first 10.\n");
		num_input_files = 15;
	}
	else {
		num_input_files = argc;
	}
	for (i = 5; i < num_input_files; i++) {
		input_files[array_idx] = argv[i];
		array_idx += 1;
	}


	// Creating requester threads
	pthread_t requester_threads[num_request];

	// create requester arguments 
	struct thread_args *t_args[num_request];

	for (i = 0; i < num_request; i++) {
		t_args[i] = malloc(sizeof(struct thread_args));
		t_args[i]->input_file_names = malloc(MAX_INPUT_FILES * sizeof(char*));
		int j;
		for (j = 0; j < MAX_INPUT_FILES; j++) {
			t_args[i]->input_file_names[j] = input_files[j];
		}

		t_args[i]->service_file = requester_log;
		t_args[i]->num_requester_threads = num_request;
		t_args[i]->num_files_serviced = 0;
		t_args[i]->num_added = &added;
		t_args[i]->num_removed = &removed;
		t_args[i]->req_exists = &req_exists;

		t_args[i]->service_file_mutex = &service_file_mutex;
		t_args[i]->results_file_mutex = &results_file_mutex;
		t_args[i]->mutex = &mutex;

		t_args[i]->space_avail = &space_avail;
		t_args[i]->items_avail = &items_avail;

		t_args[i]->buffer = array;
		t_args[i]->buf_index = &array_index;
	}

	//creating resolver threads
	pthread_t resolver_threads[num_resolve];

	//creating resolver arguments
	struct thread_args *t1_args[num_resolve];

	for (i = 0; i < num_resolve; i++){
		t1_args[i] = malloc(sizeof(struct thread_args));
		t1_args[i]->results_file = resolver_log;
		t1_args[i]->num_added = &added;
		t1_args[i]->num_removed = &removed;
		t1_args[i]->req_exists = &req_exists;

		t1_args[i]->service_file_mutex = &service_file_mutex;
		t1_args[i]->results_file_mutex = &results_file_mutex;
		t1_args[i]->mutex = &mutex;

		t1_args[i]->space_avail = &space_avail;
		t1_args[i]->items_avail = &items_avail;

		t1_args[i]->buffer = array;
		t1_args[i]->buf_index = &array_index;
	}
	
	// creating requester threads
	for (i = 0; i < num_request; i++) {
		// create individual arguments
		t_args[i]->input_file_index = i;
		t_args[i]->id = i + 1;

		//create threads
		if (pthread_create(&requester_threads[i], NULL, (void*) &request, (void *) t_args[i])) {
			printf("Failed to create requester.\n");
		}
	}

	// creating resolver threads
	for (i = 0; i < num_resolve; i++) {
		if (pthread_create(&resolver_threads[i], NULL, (void*) &resolve, (void *) t1_args[i])) {
			printf("Failed to create resolver.\n");
		}
	}

	// joining and freeing requester threads
	for (i = 0; i < num_request; i++) {
		pthread_join(requester_threads[i], 0);
		free(t_args[i]->input_file_names);
		free(t_args[i]);
	}

	// requester threads are all joined and none exist anymore
	req_exists = false;

	// joining and freeing resolver threads
	for (i = 0; i < num_resolve; i++) {
		pthread_join(resolver_threads[i], 0);
		free(t1_args[i]);
	}

	// free memory
	free(input_files);
	free(array);
	fclose(requester_log);
	fclose(resolver_log);

	// end time
	struct timeval e;
	gettimeofday(&e, NULL);
	printf("Number for requester thread = %d\n", num_request);
	printf("Number for resolver thread = %d\n", num_resolve);
	printf("Total run time: %lf\n", (e.tv_sec - s.tv_sec) + ((e.tv_usec - s.tv_usec)/1000000.0));

	return 0;
}