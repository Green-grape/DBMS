#include "trx_manager.h"

pthread_mutex_t transaction_manager_mutex=PTHREAD_MUTEX_INITIALIZER;

int global_id=0;

transaction_t* head_transaction=NULL;

void init_transaction(transaction_t* transaction){
	transaction->id=global_id;
	transaction->head=NULL;
	transaction->tail=NULL;
	transaction->back_up_head=NULL;
	transaction->back_up_tail=NULL;
	pthread_mutex_init(&(transaction->transaction_mutex),NULL);
}

//hash
void add_transaction(transaction_t* transaction){
	HASH_ADD(h2, head_transaction, id, sizeof(int), transaction);
}

transaction_t* find_transaction(transaction_t* transaction, int id){
	pthread_mutex_lock(&transaction_manager_mutex);
	int trx_id=id;
	HASH_FIND(h2, head_transaction, &trx_id, sizeof(int), transaction);
	pthread_mutex_unlock(&transaction_manager_mutex);
	return transaction;
}

void set_transaction(transaction_t* ori, transaction_t* src){
	HASH_REPLACE(h2, head_transaction, id, sizeof(int), src, ori);
}

void delete_transaction(transaction_t* transaction){
	pthread_mutex_lock(&transaction_manager_mutex);
	HASH_DELETE(h2, head_transaction, transaction);
	pthread_mutex_unlock(&transaction_manager_mutex);
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

int trx_trx_abort(int trx_id){
	transaction_t* trx=NULL;
	trx=find_transaction(trx, trx_id);
	if(trx==NULL){
		return 0;
	}
	transaction_back_up_t* cur=trx->back_up_tail;
	while(cur!=NULL){//first roll back data
		db_roll_back(cur);
		cur=cur->prev;
	}
	pthread_mutex_lock(&lock_table_mutex);
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
	pthread_mutex_unlock(&lock_table_mutex);
	delete_transaction(trx);
	printf("trx_id:%d abort\n", trx_id);
	return trx_id;
}

int trx_trx_commit(int trx_id){
	pthread_mutex_lock(&lock_table_mutex);
	int res_trx_id=trx_id;
	transaction_t* transaction=NULL;
	transaction=find_transaction(transaction, trx_id);
	//printf("transaction %d: %p\n", trx_id, transaction);
	if(transaction==NULL){
		printf("no transaction trx_id:%d\n",trx_id);
		pthread_mutex_unlock(&lock_table_mutex);
		return 0;
	}
	while(transaction->head!=NULL){//release lock in trx
		record_hash_t* temp=transaction->head->sentinel;
		int res=lock_release(transaction->head);
		
		if(res==-1){
			//printf("no lock found!\n");
			pthread_mutex_unlock(&lock_table_mutex);
			return 0;
		}
	}
	//printf("\n");
	//delete_transaction(transaction);
	pthread_mutex_unlock(&lock_table_mutex);
	//printf("trx_id:%d commit\n", trx_id);
	return res_trx_id;
}

void trx_insert_backup(int table_id, int64_t pagenum, int idx, char* back_up, int trx_id){
	transaction_t* trx=find_transaction(trx, trx_id);
	if(trx==NULL){
		printf("null trx trx_id:%d\n", trx_id);
		return;
	}
	transaction_back_up_t* trx_backup=(transaction_back_up_t*)malloc(sizeof(transaction_back_up_t));
	memset(trx_backup, 0, sizeof(transaction_back_up_t));
	trx_backup->table_id=table_id;
	trx_backup->pagenum=pagenum;
	trx_backup->idx=idx;
	trx_backup->next=NULL;
	trx_backup->prev=NULL;
	strcpy(trx_backup->back_up, back_up);
	if(trx->back_up_head==NULL){
		trx->back_up_head=trx_backup;
		trx->back_up_tail=trx_backup;
	}else{
		trx->back_up_tail->next=trx_backup;
		trx_backup->prev=trx->back_up_tail;
		trx->back_up_tail=trx_backup;
	}
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