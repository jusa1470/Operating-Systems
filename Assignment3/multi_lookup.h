#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

struct thread_args {
	FILE *service_file;
	FILE *results_file;
	int num_files_serviced;
	int num_requester_threads;
	char **input_file_names;
	int input_file_index;
	int id;
	char **buffer;
	int *buf_index;
	bool *req_exists;
	int *num_removed;
	int *num_added;

	pthread_mutex_t *service_file_mutex;
	pthread_mutex_t *results_file_mutex;
	pthread_mutex_t *mutex;
	sem_t *space_avail;
	sem_t *items_avail;
};

void requester(struct thread_args *args);
void resolve(struct thread_args *args);