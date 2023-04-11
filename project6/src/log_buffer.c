#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define DATALENGTH 120
#define NORMAL_LOG_SIZE 28
#define UPDATE_LOG_SIZE 288
#define COMPENSATE_LOG_SIZE 296
#define BEGIN 0
#define UPDATE 1
#define COMMIT 2
#define ROLLBACK 3
#define COMPENSATE 4

int curLSN=1;

typedef struct normal_log_inf_t{
	int log_size;
	int64_t LSN;
	int64_t prevLSN;
	int trx_id;
	int type;
}normal_log_inf_t;

typedef struct add_log_inf_t{
	int table_id;
	int64_t pagenum;
	int offset;
	int data_length;
	char old_image[DATALENGTH];
	char new_image[DATALENGTH];
}add_log_inf_t;

typedef struct log_buffer_t{
	struct log_buffer_t* prev;
	struct log_buffer_t* next;
	normal_log_inf_t* normal_log_inf;
	add_log_inf_t* add_log_inf;
	int64_t next_undo_LSN;
}log_buffer_t;

typedef struct log_buffer_header_t{
	log_buffer_t* head;
	log_buffer_t* tail;
	int used_buffer;
	int total_buffer;
	int64_t flushedLSN;
}log_buffer_header_t;

log_buffer_t* log_buffers=NULL;
log_buffer_header_t* log_buffer_header=NULL;
pthread_mutex_t log_buffer_manager_mutex=PTHREAD_MUTEX_INITIALIZER;

void init_log_buffer(log_buffer_t* log_buffer){
	log_buffer->prev=NULL;
	log_buffer->next=NULL;
	log_buffer->normal_log_inf=NULL;
	log_buffer->add_log_inf=NULL;
	int64_t next_undo_LSN=0;
}

void init_log_buffer_header(int num_buf){
	log_buffer_header=(log_buffer_header_t*)malloc(sizeof(log_buffer_header_t));
	memset(log_buffer_header,0, sizeof(log_buffer_header_t));
	log_buffer_header->head=NULL;
	log_buffer_header->tail=NULL;
	log_buffer_header->used_buffer=0;
	log_buffer_header->total_buffer=0;
}

int log_buf_init(int num_buf){
	log_buffer_t* temp_log_buffer=(log_buffer_t*)malloc(sizeof(log_buffer_t)*num_buf);
	if(temp_log_buffer==NULL) return -1;
	log_buffers=temp_log_buffer;
	for(int i=0;i<num_buf;i++){
		memset(&(log_buffers[i]),0,sizeof(buffer_t));
		init_log_buffer(&(log_buffers[i]));
	}
}

int log_buffer_flush(){
	log_buffer_t* cur=log_buffer_header->head;
	while(cur!=NULL){
		int log_size=cur->normal_log_inf->log_size;
		if(log_size==NORMAL_LOG_SIZE){

		}
	}
	log_buffer_header->used_buffer=0;
}

int using_log_buffer(int log_size, int log_type, int trx_id, add_log_inf_t* add_log_inf, int64_t next_undo_LSN){
	if(log_buffer_header->used_buffer==log_buffer_header->total_buffer){//buffer not full
		log_buffer_flush();
	}
	switch(log_type){
	case :
	}
}