#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "file.h"
#include "lock_table.h"

#define MAX_TABLE_ID 10
#define head_buf 0
#define root_buf 1
#define true 1
#define false 0
#define WRITE 1
#define READ 0

//structer

#pragma pack(1)

typedef struct trx_saved_data_t{
	int trx_id;
	page_t saved_frame;
	int saved_isDirty;
	struct trx_saved_data_t* prev;
	struct trx_saved_data_t* next;
}trx_saved_data_t;

typedef struct buffer_t
{
	page_t frame;
	int table_id;
	pagenum_t pagenum;
	int isDirty;
	trx_saved_data_t* saved_data_head;
	trx_saved_data_t* saved_data_tail;
	pthread_mutex_t page_mutex;
	struct buffer_t* prev_buffer;
	struct buffer_t* next_buffer;
}buffer_t;

typedef struct buffer_header_t{
	buffer_t* head;
	buffer_t* tail;
	int used_buffer;
	int total_buffer;
}buffer_header_t;


//GLOBAL
extern int error_check;
extern buffer_t* buffers;
extern buffer_header_t* buffer_header;
extern pthread_mutex_t buffer_manager_mutex;

//utility
void init_buf(buffer_t* buffer);
void init_buf_header(int num_buf);
int buf_init_db(int num_buf);
void buf_set_root_page_num(int table_id, pagenum_t pagenum);
pagenum_t buf_get_root_page_num(int table_id, int trx_id);
int make_free_buffer();
buffer_t* get_buffer(int table_id, pagenum_t pagenum);
pagenum_t buf_get_number_of_pages(int table_id);
buffer_t* using_buffer(int table_id,pagenum_t pagenum,int mode);
int buf_close_table(int table_id);
int buf_shutdown_db();
void print_buf();
void buf_file_initialize(int table_id);

//Read
int buf_read_page(int table_id, pagenum_t pagenum, page_t* dest, int trx_id);
int64_t buf_read_root_page(int table_id, page_t*dest, int trx_id);

//Write
int buf_write_page(int table_id, pagenum_t pagenum, const page_t* src, int trx_id);
int buf_write_root_page(int table_id, pagenum_t pagenum, const page_t* src);

//free & alloc
void buf_free_page(int table_id, pagenum_t pagenum);
pagenum_t buf_alloc_page(int table_id);

//trx_buffer control
void init_trx_saved_data(trx_saved_data_t* saved_data);
trx_saved_data_t* find_trx_saved_data(buffer_t* buffer, int trx_id);
void insert_trx_saved_data(buffer_t* buffer, trx_saved_data_t* new_data);
void delete_trx_saved_data(buffer_t* buffer, trx_saved_data_t* tar_data);
int buf_commit_done(int table_id, pagenum_t pagenum, int trx_id, int mode);
int buf_abort_done(int table_id, pagenum_t pagenum, int trx_id, int mode);


#endif