#include "lock_table.h"

pthread_mutex_t lock_table_mutex=PTHREAD_MUTEX_INITIALIZER; 

record_hash_t* head_record=NULL;

//int ac=0;
//util
void init_record_hash(int table_id, int64_t key, record_hash_t* record_hash){
	if(record_hash!=NULL){
		record_hash->key.table_id=table_id;
		pthread_mutex_init(&(record_hash->mutex), NULL);
		record_hash->key.key=key;
		record_hash->head=NULL;
		record_hash->tail=NULL;
	}
}

void init_lock(record_hash_t* record_hash, lock_t* lock,int lock_mode, int trx_id){
	if(lock!=NULL){
		lock->trx_id=trx_id;
		lock->flag=0;
		lock->prev=NULL;
		lock->next=NULL;
		lock->trx_prev=NULL;
		lock->trx_next=NULL;
		lock->sentinel=record_hash;
		lock->lock_mode=lock_mode;
		pthread_cond_init(&(lock->cond),NULL);
	}
}

void print_hash(record_hash_t* record){
	lock_t* cur=record->head;
	printf("\nthread: ");
	while(cur!=NULL){
		printf("%ld ", cur->trx_id);
		cur=cur->next;
	}
	printf("\n");
}

//hash
void add_record(record_hash_t* src){
	HASH_ADD(h1, head_record, key, sizeof(record_key_t), src);
}

record_hash_t* find_record(int table_id, int64_t key, record_hash_t* dest){
	record_key_t record_key;
	memset(&record_key, 0, sizeof(record_key_t));
	record_key.table_id=table_id;
	record_key.key=key;
	HASH_FIND(h1, head_record, &record_key, sizeof(record_key_t),dest);
	return dest;
}

void set_record(record_hash_t* ori, record_hash_t* src){
	HASH_REPLACE(h1, head_record, key, sizeof(record_key_t), src, ori);
	//free(ori);
}

void insert_lock(record_hash_t* record, lock_t* lock){
	//printf("1 thread: %ld, head:%p tail: %p lock:%p\n", pthread_self(), record->head, record->tail,lock);
	if(record->head==NULL && record->tail==NULL){
		record->head=lock;
		record->tail=lock;
	}
	else{
		record->tail->next=lock;
		lock->prev=record->tail;
		record->tail=lock;
	}
	///printf("2 thread: %ld, head:%p tail: %p lock:%p\n", pthread_self(), record->head, record->tail,lock);
}
//1:need to wake up, 0:don't need to wake up
void delete_lock(record_hash_t* record, lock_t* lock)
{
	//printf("delete lock trx:%d\n", lock->trx_id);
	// //printf("1 thread: %ld, head:%p tail: %p lock:%p\n", pthread_self(), record->head, record->tail,lock);
	// if(record->head==lock){
	// 	//printf("thread: %ld, head delete\n", pthread_self());
	// 	record->head=lock->next;
	// 	if(record->head!=NULL){
	// 		record->head->prev=NULL;
	// 	}
	// 	if(record->tail==lock) record->tail=NULL;
	// }
	// else if(record->tail==lock){
	// 	record->
	// }
	// //printf("2 thread: %ld, head:%p tail: %p lock:%p\n", pthread_self(), record->head, record->tail,lock);
	if(lock->prev!=NULL) lock->prev->next=lock->next;
	if(lock->next!=NULL) lock->next->prev=lock->prev;
	if(record->head==lock) record->head=lock->next;
	if(record->tail==lock) record->tail=lock->prev;
}

//lock_table
int is_need_waiting(record_hash_t* record, int trx_id, int lock_mode){
	//flag 1:acquired 0:waiting
	if(record->head==NULL || record->tail==NULL) return 0;

	if(record->tail->trx_id==trx_id){
		return 0;
	}
	if(record->tail->flag==1){
		if(record->tail->lock_mode==SHARED && lock_mode==SHARED) return 0;
		else return 1;
	}
	return 1;
}

lock_t* wake_up_target_lock(record_hash_t* record, lock_t* lock){
	//flag 1:acquired 0:waiting
	if(record->tail==lock) return NULL;

	if(record->head==lock){
		return lock->next;
	}else{
		if(lock->prev->flag==1){
			if(lock->prev->lock_mode==SHARED && lock->lock_mode==EXCLUSIVE && lock->next->lock_mode==SHARED) return lock->next;
		}
		return NULL;
	}
}

int is_need_unlock(record_hash_t* record, lock_t* lock){
	if(record->head==lock) return 1;
	return 0;
}

int is_need_lock(record_hash_t* record, int lock_mode, lock_t* new_lock){
	// if(!(lock_mode==SHARED && record->head!=NULL)){
	// 	if(record->tail->prev!=NULL){
	// 		return !(record->tail->prev->trx_id==record->tail->trx_id);
	// 	}
	// 	return 1;
	// }
	return 0;
}

void print_waiting_list(record_hash_t* record){
	lock_t* cur=record->head;
	while(cur!=NULL){
		printf("%d/m:%d/f:%d ", cur->trx_id, cur->lock_mode, cur->flag);
		cur=cur->next;
	}
	printf("\n");
}

int init_lock_table(){
	return 0;
}

/*
First, find dead lock with already acquired condition (It can be strange, 
but my wait-for-graph searching method can do both of them at one time).
If condition satisfied one of above conditions, return NULL(dead lock), existing lock
Second, make new lock and insert to trx & record.
Third, make sleep or locking with is_need_waiting function
*/
lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode, int isUpgrade){
	pthread_mutex_lock(&transaction_manager_mutex);
	pthread_mutex_lock(&lock_table_mutex);
	record_hash_t* record=NULL;
	if(isUpgrade) return NULL;
	//printf("acquire thread: %ld ac:%d table_id: %d key: %ld\n", pthread_self(), ac, table_id, key);
	int dead_lock_op=lock_find_deadlock(table_id, key, trx_id, lock_mode);
	//printf("\ntrx: %d ,dead_lock_op:%d\n", trx_id, dead_lock_op);
	switch(dead_lock_op){
		case 1: //dead lock
			trx_abort(trx_id);
			pthread_mutex_unlock(&transaction_manager_mutex);
			pthread_mutex_unlock(&lock_table_mutex);
			return NULL;
		case 2: //already acquired
			record=find_record(table_id, key, record);
			lock_t* cur=record->head;
			while(cur!=NULL){
				if(cur->flag==1 && cur->trx_id==trx_id && cur->lock_mode==lock_mode){
					pthread_mutex_unlock(&transaction_manager_mutex);
					pthread_mutex_unlock(&lock_table_mutex);
					return cur;
				}
				cur=cur->next;
			}
			pthread_mutex_unlock(&transaction_manager_mutex);
			pthread_mutex_unlock(&lock_table_mutex);
			return NULL;
	}
	
	record=find_record(table_id, key, record);
	if(record==NULL){//first insert to hash
		//printf("new record\n");
		record=(record_hash_t*)malloc(sizeof(record_hash_t));
		memset(record, 0, sizeof(record_hash_t));
		init_record_hash(table_id, key, record);
		add_record(record);
	}
	
	record_hash_t* changed_record=record;
	lock_t* new_lock=(lock_t*)malloc(sizeof(lock_t));
	memset(new_lock, 0, sizeof(lock_t));
	init_lock(changed_record, new_lock,lock_mode,trx_id);

	transaction_t* transaction=NULL;
	//printf("trx_lock");
	transaction=find_transaction(transaction, trx_id);
	if(transaction!=NULL){
		transaction_t* changed_trx=transaction;
		add_lock_to_trx(changed_trx, new_lock);
		set_transaction(transaction, changed_trx);
	}
	pthread_mutex_unlock(&transaction_manager_mutex);

	//flag 1:acquired 0:waiting
	if(is_need_waiting(changed_record, trx_id, lock_mode) || dead_lock_op==3){
		printf("trx:%d is waiting:page:%ld \n", trx_id, key);
		print_waiting_list(record);
		insert_lock(changed_record, new_lock);
		set_record(record, changed_record);
		lock_t* before_cur=NULL;
		if(dead_lock_op==3){
			lock_t* cur=new_lock->prev;
			while(cur!=NULL && cur->trx_id==trx_id){
				cur->flag=0;
				before_cur=cur;
				cur=cur->prev;
			}
		}
		if(dead_lock_op==3){
			while(before_cur->flag==0){
				pthread_cond_wait(&(before_cur->cond), &lock_table_mutex);
			}
		}
		else{
			while(new_lock->flag==0){
				pthread_cond_wait(&(new_lock->cond),&lock_table_mutex);
			}
		}
		//printf("thread: %ld signal on\n", pthread_self());
		//if(record->head==new_lock) pthread_mutex_lock(&(record->mutex));
		if(dead_lock_op==3){
			if(before_cur==before_cur->sentinel->head){
				lock_t* cur=before_cur;
				while(cur->next!=NULL){
					if(cur->lock_mode==SHARED && cur->next->lock_mode==SHARED) cur->next->flag=1;
					if(cur->sentinel->head==cur && cur->trx_id==cur->next->trx_id) cur->next->flag=1;
					cur=cur->next;
				}
			}
			if(before_cur->next!=NULL && before_cur->lock_mode==SHARED && before_cur->next->lock_mode==SHARED){
				pthread_cond_signal(&(before_cur->next->cond));
			}
		}
		else{
			if(new_lock==new_lock->sentinel->head){
				lock_t* cur=new_lock;
				while(cur->next!=NULL){
					if(cur->lock_mode==SHARED && cur->next->lock_mode==SHARED) cur->next->flag=1;
					cur=cur->next;
				}
			}
			if(new_lock->next!=NULL && new_lock->lock_mode==SHARED && new_lock->next->lock_mode==SHARED){
				pthread_cond_signal(&(new_lock->next->cond));
			}
		}
	}else{
		//printf("trx:%d is working\n", trx_id);
		new_lock->flag=1;
		insert_lock(changed_record, new_lock);
		set_record(record, changed_record);
		//print_hash(changed_record);
		if(is_need_lock(changed_record, lock_mode, new_lock)) pthread_mutex_lock(&(changed_record->mutex));
	}

	pthread_mutex_unlock(&lock_table_mutex);
	return new_lock;
}

//lock release in trx & record and wake up if need
int lock_release(lock_t* lock_obj){
	record_hash_t* record=NULL;
	transaction_t* transaction=NULL;
	record=find_record(lock_obj->sentinel->key.table_id, lock_obj->sentinel->key.key, record);
	
	transaction=find_transaction(transaction, lock_obj->trx_id);
	if(record==NULL){
		return -1;
	}
	
	record_hash_t* changed_record=record;
	transaction_t* changed_trx=transaction;
	lock_t* target_lock=wake_up_target_lock(changed_record, lock_obj);
	int c2=is_need_unlock(changed_record, lock_obj);

	delete_lock(changed_record, lock_obj);
	delete_lock_from_trx(changed_trx, lock_obj);
	set_record(record, changed_record);
	set_transaction(transaction, changed_trx);

	if(c2) pthread_mutex_unlock(&(changed_record->mutex));
	if(target_lock!=NULL){
		target_lock->flag=1;
		pthread_cond_signal(&(target_lock->cond));
	}
	free(lock_obj);
	return 0;
}

int lock_upgrade(int table_id, int64_t key, int trx_id, int lock_mode){

}

//1:deadlock 0:no deadlock 
//find by 2 method: same trx in record & mutul waiting
int lock_find_deadlock(int table_id,int64_t key, int trx_id, int lock_mode){
	
	record_hash_t* record=NULL;
	transaction_t* trx=NULL;
	record=find_record(table_id, key, record);

	if(record==NULL) {
		return 0;
	}

	lock_t* cur=record->head;
	int is_same_trx_id_exist=0;
	//printf("trx_id: %d find dead lock in %ld key", trx_id, key);
	while(cur!=NULL){
		//printf("-> %d/%d", cur->trx_id, cur->lock_mode);
		transaction_t* temp_trx=find_transaction(temp_trx, cur->trx_id);
		if(cur->trx_id==trx_id){
			lock_t* temp=cur;
			while(temp!=NULL && temp->trx_id==trx_id){
				if(temp->flag==1 && (temp->lock_mode==lock_mode || temp->lock_mode==EXCLUSIVE)) return 2;
				temp=temp->next;
			}
			if(temp==NULL && record->head->trx_id!=trx_id){//no different trx in between
				return 3;
			}
			if(!(cur->sentinel->head->trx_id==trx_id && cur->sentinel->tail->trx_id==trx_id)){
				lock_t* s=cur->sentinel->head;
				while(s!=NULL){
					printf("%d/m:%d/f:%d ", s->trx_id, s->lock_mode, s->flag);
					s=s->next;
				}
				printf("\n");
				printf("trx_id: %d is abort! (same trx abort) in page:%ld\n", trx_id, key);
				return 1;
			}
		}
		if(temp_trx==NULL){
			cur=cur->next;
			continue;
		}
		lock_t* trx_lock=temp_trx->head;
		while(trx_lock!=NULL){
			if(trx_lock->flag!=1){
				lock_t* temp_lock=trx_lock->prev;
				while(temp_lock!=NULL){
					if(temp_lock->trx_id==trx_id){
						lock_t* s=trx_lock->prev;
						while(s!=NULL){
							printf("%d/m:%d/f:%d ", s->trx_id, s->lock_mode, s->flag);
							s=s->prev;
						}
						printf("\n");
						printf("trx_id: %d is abort! (dead lock abort) in page:%ld\n", trx_id, key);
						return 1;
					}
					temp_lock=temp_lock->prev;
				}
			}
			trx_lock=trx_lock->trx_next;
		}
		cur=cur->next;
	}
	//printf("\n");
	return 0;
}