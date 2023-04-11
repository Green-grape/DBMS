#include "trx_manager.h"
#include "buffer.h"
#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#define EXCLUSIVE 1
#define SHARED 0
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "uthash.h" 

#pragma pack(1)

typedef struct record_key_t{
	int table_id;
	int64_t key;
}record_key_t;

typedef struct record_hash_t record_hash_t;

struct lock_t{
	int trx_id;
	int flag;
	int lock_mode;
	struct lock_t* prev;
	struct lock_t* next;
	struct lock_t* trx_prev;
	struct lock_t* trx_next;
	record_hash_t* sentinel;
	pthread_cond_t cond;
};

typedef struct lock_t lock_t;

struct record_hash_t{
	record_key_t key;
	pthread_mutex_t mutex;
	lock_t* head;
	lock_t* tail;
	UT_hash_handle h1;
};


//GLOBAL
extern pthread_mutex_t lock_table_mutex;

extern record_hash_t* head_record;

/*--------LOCK-TABLE----------*/

//util
void init_record_hash(int table_id, int64_t key, record_hash_t* record_hash);
void init_lock(record_hash_t* record_hash, lock_t* lock,int lock_mode, int trx_id);
void print_hash(record_hash_t* record);
void print_waiting_list(record_hash_t* record);

//hash
void add_record(record_hash_t* src);
record_hash_t* find_record(int table_id, int64_t key, record_hash_t* dest);
void insert_lock(record_hash_t* record, lock_t* lock);
void delete_lock(record_hash_t* record, lock_t* lock);
void set_record(record_hash_t* ori, record_hash_t* src);

//deadlock
int lock_find_deadlock(int table_id,int64_t key, int trx_id, int lock_mode);

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key,int trx_id, int lock_mode, int isUpgrade);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
