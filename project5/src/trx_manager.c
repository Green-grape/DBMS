#include "trx_manager.h"

pthread_mutex_t transaction_manager_mutex=PTHREAD_MUTEX_INITIALIZER;

int global_id=0;

transaction_t* head_transaction=NULL;

void init_transaction(transaction_t* transaction){
	transaction->id=global_id;
	transaction->head=NULL;
	transaction->tail=NULL;
}

//hash
void add_transaction(transaction_t* transaction){
	HASH_ADD(h2, head_transaction, id, sizeof(int), transaction);
}

transaction_t* find_transaction(transaction_t* transaction, int id){
	HASH_FIND(h2, head_transaction, &id, sizeof(int), transaction);
	return transaction;
}

void set_transaction(transaction_t* ori, transaction_t* src){
	HASH_REPLACE(h2, head_transaction, id, sizeof(int), src, ori);
}

void delete_transaction(transaction_t* transaction){
	HASH_DELETE(h2, head_transaction, transaction);
}

//lock control
void add_lock_to_trx(transaction_t* transaction, lock_t* lock){
	if(transaction->head==NULL){
		transaction->head=lock;
		transaction->tail=lock;
	}
	else{
		transaction->tail->trx_next=lock;
		lock->trx_prev=transaction->tail;
		transaction->tail=lock;
	}
}

void delete_lock_from_trx(transaction_t* transaction, lock_t* lock){
	if(lock->trx_prev!=NULL) lock->trx_prev->trx_next=lock->trx_next;
	if(lock->trx_next!=NULL) lock->trx_next->trx_prev=lock->trx_prev;
	if(transaction->head==lock) transaction->head=lock->trx_next;
	if(transaction->tail==lock) transaction->tail=lock->trx_prev;
}

//trx
int trx_trx_begin(void){
	pthread_mutex_lock(&transaction_manager_mutex);
	global_id++;
	int res=global_id;
	transaction_t* new_transaction=(transaction_t*)malloc(sizeof(transaction_t));
	memset(new_transaction, 0, sizeof(transaction_t));
	init_transaction(new_transaction);

	add_transaction(new_transaction);
	pthread_mutex_unlock(&transaction_manager_mutex);
	return res;
}

int trx_abort(int trx_id){
	transaction_t* trx=NULL;
	trx=find_transaction(trx, trx_id);
	if(trx==NULL){
		return 0;
	}
	lock_t* cur=trx->head;
	while(cur!=NULL){//first roll back data
		buf_abort_done(cur->sentinel->key.table_id, cur->sentinel->key.key, trx_id, cur->lock_mode);
		cur=cur->trx_next;
	}
	while(trx->head!=NULL){//release all lock in trx
		//printf("trx_id: %d abort page:%ld\n", trx_id, trx->head->sentinel->key.key);
		record_hash_t* temp=trx->head->sentinel;
		int res=lock_release(trx->head);
		//printf("trx_id: %d (page:%ld) abort after: ", trx_id, temp->key.key);
		//print_waiting_list(temp);
		if(res==-1){
			//printf("no lock found!\n");
			return 0;
		}
	}
	delete_transaction(trx);
	return trx_id;
}

int trx_trx_commit(int trx_id){
	pthread_mutex_lock(&transaction_manager_mutex);
	pthread_mutex_lock(&lock_table_mutex);
	int res_trx_id=trx_id;
	transaction_t* transaction=NULL;
	transaction=find_transaction(transaction, trx_id);
	//printf("transaction %d: %p\n", trx_id, transaction);
	if(transaction==NULL){
		pthread_mutex_unlock(&transaction_manager_mutex);
		return 0;
	}
	//printf("lock: ");
	lock_t* cur=transaction->head;
	while(cur!=NULL){//remove temp data of buffer
		buf_commit_done(cur->sentinel->key.table_id, cur->sentinel->key.key, trx_id, cur->lock_mode);
		cur=cur->trx_next;
	}
	while(transaction->head!=NULL){//release lock in trx
		//printf("page:%ld", transaction->head->sentinel->key.key);
		record_hash_t* temp=transaction->head->sentinel;
		int res=lock_release(transaction->head);
		//printf("trx_id: %d (page:%ld)commit after: ", trx_id, temp->key.key);
		//print_waiting_list(temp);
		if(res==-1){
			//printf("no lock found!\n");
			pthread_mutex_unlock(&transaction_manager_mutex);
			pthread_mutex_unlock(&lock_table_mutex);
			return 0;
		}
	}
	//printf("\n");
	delete_transaction(transaction);
	pthread_mutex_unlock(&transaction_manager_mutex);
	pthread_mutex_unlock(&lock_table_mutex);
	return res_trx_id;
}

void print_trx(int start_id, int end_id){
	pthread_mutex_lock(&transaction_manager_mutex);
	for(int i=start_id;i<=end_id;i++){
		transaction_t* trx=find_transaction(trx, i);
		if(trx!=NULL){
			lock_t* cur=trx->head;
			//printf("trx: %d ", i);
			while(cur!=NULL){
				// if(cur->flag==0) printf("s:%ld", cur->sentinel->key.key);
				// else printf("w:%ld", cur->sentinel->key.key);
				cur=cur->trx_next;
			}
			//printf("\n");
		}
	}
	pthread_mutex_unlock(&transaction_manager_mutex);
}