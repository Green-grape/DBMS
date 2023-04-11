#include "lock_table.h"

pthread_mutex_t lock_table_mutex=PTHREAD_MUTEX_INITIALIZER; 

//pthread_mutex_t double_lock_table_mutex=PTHREAD_MUTEX_INITIALIZER;

record_hash_t* head_record=NULL;

//int ac=0;
//util
void init_record_hash(int table_id, int64_t key, record_hash_t* record_hash){
	if(record_hash!=NULL){
		record_hash->key.table_id=table_id;
		record_hash->key.key=key;
		pthread_mutex_init(&(record_hash->mutex), NULL);
		record_hash->head=NULL;
		record_hash->tail=NULL;
	}
}

void init_lock(record_hash_t* record_hash, lock_t* lock){
	if(lock!=NULL){
		lock->thread_id=pthread_self();
		lock->flag=0;
		lock->prev=NULL;
		lock->next=NULL;
		lock->sentinel=record_hash;
		pthread_cond_init(&(lock->cond),NULL);
	}
}

void print_hash(record_hash_t* record){
	lock_t* cur=record->head;
	printf("\nthread: ");
	while(cur!=NULL){
		printf("%ld ", cur->thread_id);
		cur=cur->next;
	}
	printf("\n");
}

//hash
void add_record(record_hash_t* src){
	HASH_ADD(hh, head_record, key, sizeof(record_key_t), src);
}

record_hash_t* find_record(int table_id, int64_t key, record_hash_t* dest){
	record_key_t record_key;
	memset(&record_key, 0, sizeof(record_key_t));
	record_key.table_id=table_id;
	record_key.key=key;
	HASH_FIND(hh, head_record, &record_key, sizeof(record_key_t),dest);
	return dest;
}

void set_record(record_hash_t* ori, record_hash_t* src){
	HASH_REPLACE(hh, head_record, key, sizeof(record_key_t), src, ori);
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

void delete_lock(record_hash_t* record, lock_t* lock)
{
	//printf("1 thread: %ld, head:%p tail: %p lock:%p\n", pthread_self(), record->head, record->tail,lock);
	if(record->head==lock){//need to wake up
		//printf("thread: %ld, head delete\n", pthread_self());
		record->head=lock->next;
		if(record->head!=NULL){
			record->head->prev=NULL;
		}
		if(record->tail==lock) record->tail=NULL;
	}
	else{
		//printf("no head delete\n");
	}
	//printf("2 thread: %ld, head:%p tail: %p lock:%p\n", pthread_self(), record->head, record->tail,lock);

}

//lock_table
int init_lock_table(){
	return 0;
}

lock_t* lock_acquire(int table_id, int64_t key){
	pthread_mutex_lock(&lock_table_mutex);
	//printf("acquire thread: %ld ac:%d table_id: %d key: %ld\n", pthread_self(), ac, table_id, key);
	record_hash_t* record=NULL;
	record=find_record(table_id, key, record);
	if(record==NULL){//first insert to hash
		//printf("new record\n");
		record=(record_hash_t*)malloc(sizeof(record_hash_t));
		memset(record, 0, sizeof(record_hash_t));
		init_record_hash(table_id, key, record);
		add_record(record);
	}
	//ac++;
	//printf("head record:%p\n",head_record);
	//printf("%p\n", record);
	record_hash_t* changed_record=record;
	lock_t* new_lock=(lock_t*)malloc(sizeof(lock_t));
	memset(new_lock, 0, sizeof(lock_t));
	init_lock(changed_record, new_lock);

	if(changed_record->head==NULL && changed_record->tail==NULL){//no record
		//printf("not wait\n");
		insert_lock(changed_record, new_lock);
		set_record(record, changed_record);
		//print_hash(changed_record);
		pthread_mutex_lock(&(changed_record->mutex));
	}
	else{
		//printf("thread: %ld wait, new_lock_flag:%d\n", pthread_self(), new_lock->flag);
		insert_lock(record, new_lock);
		set_record(record, changed_record);
		//print_hash(changed_record);
		while(new_lock->flag==0){
			pthread_cond_wait(&(new_lock->cond),&lock_table_mutex);
		}
		//printf("thread: %ld signal on\n", pthread_self());
		pthread_mutex_lock(&(changed_record->mutex));
	}
	pthread_mutex_unlock(&lock_table_mutex);
	return new_lock;
}

int lock_release(lock_t* lock_obj){
	pthread_mutex_lock(&lock_table_mutex);
	//pthread_mutex_lock(&double_lock_table_mutex);
	//printf("release thread: %ld ac:%d table_id: %d key: %ld\n", pthread_self(), ac, lock_obj->sentinel->key.table_id, lock_obj->sentinel->key.key);
	//ac++;
	record_hash_t* record=NULL;
	//printf("record:%p\n", lock_obj->sentinel);
	record=find_record(lock_obj->sentinel->key.table_id, lock_obj->sentinel->key.key, record);
	if(record==NULL) return -1;
	
	//printf("%p\n", record);
	record_hash_t* changed_record=record;
	delete_lock(changed_record, lock_obj);
	set_record(record, changed_record);
	//print_hash(changed_record);
	pthread_mutex_unlock(&(changed_record->mutex));
	if(changed_record->head!=NULL){
		changed_record->head->flag=1;
		pthread_cond_signal(&(changed_record->head->cond));
	}
	free(lock_obj);
	//pthread_mutex_unlock(&double_lock_table_mutex);
	pthread_mutex_unlock(&lock_table_mutex);
	return 0;
}
