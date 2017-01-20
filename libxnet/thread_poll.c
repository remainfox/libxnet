#include "common.h"



void *worker(void *arg)
{
	thread_pool_t* pool = (thread_pool_t*)arg;

	while (!pool->stop){
		
		sem_wait(&pool->sem);
		pthread_mutex_lock(&pool->locker);

		if (queue_empty(&pool->conn_head))
		{
			pthread_mutex_unlock(&pool->locker);
			continue;
		}

		queue_t* curr_node = queue_head(&pool->conn_head);
		queue_remove(curr_node);
		pthread_mutex_unlock(&pool->locker);
		client_conn_t * conn = (client_conn_t *) ((u_char *) curr_node - ((size_t) &((client_conn_t *)0)->head));
		if (conn == NULL)
		{
			printf("error: http_conn is NULL.");
			continue;
		}
		
		//process(conn);
		
	}
	
	return NULL;	

}


thread_pool_t* xnet_create_threadpool(int thread_number, int max_requests)
{
	if ((thread_number <= 0) || (max_requests <= 0)){
		return NULL;
	}

	thread_pool_t * pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
	if (pool == NULL){
		return NULL;
	}

	pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_number);
	if (pool->threads == NULL){
		free(pool);
		return NULL;
	}

	pool->conn_number = 0;
	pool->thread_number = thread_number;
	pool->max_resquests = max_requests;
	queue_init(&pool->conn_head);

	if (sem_init(&pool->sem, 0, 0) != 0){
		free(pool->threads);
		free(pool);
		return NULL;
	}

	if (pthread_mutex_init(&pool->locker, NULL) != 0){
		sem_destroy(&pool->sem);
		free(pool->threads);
		free(pool);
		return NULL;
	}

	int i=0;
	for (; i<thread_number; ++i){
		//printf("create the %dth thread\n", i);
		if (pthread_create(&pool->threads[i], NULL, worker, pool) != 0){
			goto FAILED;
		}

		if (pthread_detach(pool->threads[i])){
			goto FAILED;
		}
	}

	return pool;

	
FAILED:
	sem_destroy(&pool->sem);
	pthread_mutex_destroy(&pool->locker);
	free(pool->threads);
	free(pool);
	return NULL;

}


