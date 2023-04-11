#ifndef __TRX_MANAGER_H__
#define __TRX_MANAGER_H__

#include <stdlib.h>
#include <stdint.h>
#include "lock_table.h"

typedef struct transaction_t{
	int64_t id;
	lock_t* head;
	lock_t* tail;
	UT_hash_handle h2;
}transaction_t;

//GLOBAL
extern pthread_mutex_t transaction_manager_mutex;

extern int global_id;

extern transaction_t* head_transaction;

//util
void init_transaction(transaction_t* transaction);

//hash
void add_transaction(transaction_t* transaction);
transaction_t* find_transaction(transaction_t* transaction, int trx_id);
void set_transaction(transaction_t* ori, transaction_t* src);
void detele_transaction(transaction_t* transaction);

//lock control in trx
void add_lock_to_trx(transaction_t* transaction, lock_t* lock);
void delete_lock_from_trx(transaction_t* transaction, lock_t* lock);

//trx
int trx_begin(void);
int trx_abort(int trx_id);
int trx_commit(int trx_id);

#endif 