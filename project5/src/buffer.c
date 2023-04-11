#include "buffer.h"

buffer_t* buffers=NULL;
buffer_header_t* buffer_header=NULL;
pthread_mutex_t buffer_manager_mutex=PTHREAD_MUTEX_INITIALIZER;

void init_buf(buffer_t* buffer){
	if(buffer!=NULL){
		buffer->table_id=0;
		buffer->pagenum=0;
		buffer->isDirty=false;
		buffer->saved_data_head=NULL;
		buffer->saved_data_tail=NULL;
		pthread_mutex_init(&(buffer->page_mutex),NULL);
		buffer->prev_buffer=NULL;
		buffer->next_buffer=NULL;
	}
}

void init_buf_header(int num_buf){
	if(buffer_header==NULL){
		buffer_header=(buffer_header_t*)malloc(sizeof(buffer_header_t));
		memset(buffer_header,0,sizeof(buffer_header_t));
		buffer_header->used_buffer=0;
		buffer_header->total_buffer=num_buf;
		buffer_header->head=NULL;
		buffer_header->tail=NULL;
	}
}

int buf_init_db(int num_buf){
	buffer_t* temp_buffers=(buffer_t*)malloc(sizeof(buffer_t)*num_buf);
	if(temp_buffers==NULL) return -1;
	buffers=temp_buffers;
	for(int i=0;i<num_buf;i++){
		memset(&((buffers)[i]),0,sizeof(buffer_t));
		init_buf(&((buffers)[i]));
	}
	init_buf_header(num_buf);
	return 0;
}
/*
make free buffer page, If buffer isn't full counting up buffer size,
else make free buffer using LRU policy
*/
int make_free_buffer(){
	if(buffer_header->used_buffer<buffer_header->total_buffer){
		buffer_header->used_buffer++;
		return 1;
	}
	else{
		buffer_t* cur=buffer_header->tail;
		while(cur!=NULL){
			if(pthread_mutex_trylock(&(cur->page_mutex))==0){
				if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
				if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
				if(cur->isDirty){
					file_write_page(cur->table_id,cur->pagenum,&(cur->frame));
				}
				if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
				if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
				pthread_mutex_unlock(&(cur->page_mutex));
				free(cur);
				return 1;
			}
			cur=cur->prev_buffer;
		}
		return -1;
	}
}

/*
get buffer by linear search, if it exist returning buffer,
else return NULL.
*/
buffer_t* get_buffer(int table_id,pagenum_t pagenum){
	buffer_t* cur=buffer_header->head;
	while(cur!=NULL){
		if(cur->table_id==table_id && cur->pagenum==pagenum) return cur;
		cur=cur->next_buffer;
	}
	return NULL;
}
/*
using_buffer with table_id & pagenum. If corresponding buffer is in buffer array
moving head, else read page and move to head.
*/

buffer_t* using_buffer(int table_id,pagenum_t pagenum,int mode){
	pthread_mutex_lock(&buffer_manager_mutex);
	buffer_t* temp;
	error_check=false;
	buffer_t* buffer=get_buffer(table_id,pagenum);
	if(buffer==NULL){
		int res=make_free_buffer();
		if(res==-1) {
			error_check=true;
			//printf("no more free page %d\n",buffer_header->used_buffer);
			//printf("buffer->head:%p, buffer->tail:%p\n",buffer_header->head, buffer_header->tail);
			pthread_mutex_unlock(&(buffer_manager_mutex));
			return NULL;
		}
		//print_buf();
		buffer_t* new_buffer=(buffer_t*)malloc(sizeof(buffer_t));
		memset(new_buffer,0,sizeof(buffer_t));
		init_buf(new_buffer);

		if(mode==READ) file_read_page(table_id,pagenum,&(new_buffer->frame));
		new_buffer->table_id=table_id;
		new_buffer->pagenum=pagenum;
		new_buffer->next_buffer=buffer_header->head;
		if(buffer_header->head!=NULL) buffer_header->head->prev_buffer=new_buffer;
		buffer_header->head=new_buffer;
		if(buffer_header->tail==NULL) buffer_header->tail=new_buffer;
	}else{
		if(buffer!=buffer_header->head){
			if(buffer==buffer_header->tail){
				buffer_header->head->prev_buffer=buffer;
				buffer->next_buffer=buffer_header->head;
				buffer_header->tail=buffer->prev_buffer;
				if(buffer_header->tail!=NULL) buffer_header->tail->next_buffer=NULL;
				buffer->prev_buffer=NULL;
				buffer_header->head=buffer;
			}else{
				buffer->prev_buffer->next_buffer=buffer->next_buffer;
				buffer->next_buffer->prev_buffer=buffer->prev_buffer;
				buffer_header->head->prev_buffer=buffer;
				buffer->next_buffer=buffer_header->head;
				buffer->prev_buffer=NULL;
				buffer_header->head=buffer;
			}
		}	
	}
	pthread_mutex_lock(&(buffer_header->head->page_mutex));
	temp=buffer_header->head;
	pthread_mutex_unlock(&(buffer_manager_mutex));
	return temp;
}

int buf_read_page(int table_id, pagenum_t pagenum, page_t* dest,int trx_id){
	//printf("read trx:%d page:%ld\n", trx_id,pagenum);
	buffer_t* buffer=using_buffer(table_id,pagenum,READ);
	lock_t* lock_res=NULL;
	if(buffer!=NULL){
		//buffer_header->head->isPinned=false;
		pthread_mutex_unlock(&(buffer->page_mutex));
		if(trx_id>0) lock_res=lock_acquire(table_id, pagenum, trx_id, READ, 0);
		if(lock_res==NULL && trx_id!=0){//abort
			//printf("res:NULL in trx:%d\n", trx_id);
			return -2;
		}
		buffer=using_buffer(table_id, pagenum, READ);
		if(buffer!=NULL){
			(*dest)=buffer->frame;
		}
		pthread_mutex_unlock(&(buffer->page_mutex));
		return 0;	
	}
	if(buffer==NULL) {
		error_check=true;
	}
	return -1;
}

int buf_write_page(int table_id, pagenum_t pagenum, const page_t* src, int trx_id){
	//printf("write trx:%d page:%ld\n", trx_id,pagenum);
	buffer_t* buffer=using_buffer(table_id,pagenum,WRITE);
	lock_t* lock_res=NULL;
	if(buffer!=NULL){
		//buffer_header->head->isPinned=false;
		pthread_mutex_unlock(&(buffer->page_mutex));
		if(trx_id>0) lock_res=lock_acquire(table_id, pagenum, trx_id, WRITE, 0);
		if(lock_res==NULL && trx_id!=0){//abort
			//printf("res:NULL in trx:%d\n", trx_id);
			return -2;
		}
		buffer=using_buffer(table_id, pagenum, WRITE);
		if(buffer!=NULL){
			buffer->frame=(*src);
			buffer->isDirty=true;
			if(trx_id>0){
				trx_saved_data_t* saved_data=find_trx_saved_data(buffer, trx_id);
				if(saved_data==NULL){
					saved_data=(trx_saved_data_t*)malloc(sizeof(trx_saved_data_t));
					memset(saved_data, 0, sizeof(trx_saved_data_t));
					init_trx_saved_data(saved_data);
					saved_data->trx_id=trx_id;
					saved_data->saved_frame=buffer->frame;
					insert_trx_saved_data(buffer, saved_data);
				}
			}
			pthread_mutex_unlock(&(buffer->page_mutex));
			return 0;
		}
		return -1;
	}
	if(buffer==NULL) error_check=true;
	return -1;
}

int64_t buf_read_root_page(int table_id, page_t* dest,int trx_id){
	//printf("read trx:%d page:%ld\n", trx_id,0);
	buffer_t* buffer=using_buffer(table_id,0,READ);
	if(buffer!=NULL){
		pthread_mutex_unlock(&(buffer->page_mutex));
		lock_t* lock_res=NULL;
		if(trx_id>0) lock_res=lock_acquire(table_id, 0, trx_id, READ, 0);
		if(lock_res==NULL && trx_id!=0){//abort
			//printf("res:NULL in trx:%d\n", trx_id);
			return -2;
		}
		buffer=using_buffer(table_id,0 ,READ);
		if(buffer==NULL || buffer->frame.number_of_page==1){//no root page
			pthread_mutex_unlock(&(buffer->page_mutex));
			return -1;
		}
		pagenum_t root_page_num=buffer->frame.root_page;
		pthread_mutex_unlock(&(buffer->page_mutex));
		//rintf("read trx:%d page:%ld\n", trx_id, root_page_num);
		buffer=using_buffer(table_id,root_page_num,READ);
		if(buffer!=NULL){
			pthread_mutex_unlock(&(buffer->page_mutex));
			if(trx_id>0)lock_res=lock_acquire(table_id, root_page_num, trx_id, READ,0);
			if(lock_res==NULL && trx_id!=0){//abort
				//printf("res:NULL in trx:%d\n", trx_id);
				return -2;
			}
			buffer=using_buffer(table_id, root_page_num, READ);
			if(buffer!=NULL){
				(*dest)=buffer->frame;
				pthread_mutex_unlock(&(buffer->page_mutex));
				return root_page_num;
			}
			return -1;
		}
	}
	if(buffer==NULL) error_check=true;
	return -1;
}

int buf_write_root_page(int table_id, pagenum_t pagenum, const page_t* src){
	buffer_t* buffer=using_buffer(table_id,0,READ);
	if(buffer!=NULL){
		if(buffer->frame.number_of_page==1){
			pagenum_t page=0;
			if(buffer->isDirty){
				file_write_page(table_id,0,&(buffer->frame));
				buffer->isDirty=false;
			}
			page=file_alloc_page(table_id);
			file_read_page(table_id,0,&(buffer->frame));
			buffer->frame.root_page=page;
			buffer->isDirty=true;
			//buffer_header->head->isPinned=false;
			pthread_mutex_unlock(&(buffer->page_mutex));
		}
		// else if(pagenum!=buffer_header->head->frame.root_page){
		// 	//free root_page
		// 	if(buffer_header->head->isDirty){
		// 		file_write_page(table_id,0, &(buffer_header->head->frame));
		// 		buffer_header->head->isDirty=false;
		// 	}
		// 	file_free_page(table_id,pagenum);
		// 	file_read_page(table_id,0,&(buffer_header->head->frame));

		// 	buffer_t* buffer=get_buffer(table_id, pagenum);
		// 	if(buffer!=NULL){
		// 		if(buffer->prev_buffer!=NULL) buffer->prev_buffer->next_buffer=buffer->next_buffer;
		// 		if(buffer->next_buffer!=NULL) buffer->next_buffer->prev_buffer=buffer->prev_buffer;
		// 		if(buffer==buffer_header->head) buffer_header->head=buffer->next_buffer;
		// 		if(buffer==buffer_header->tail) buffer_header->tail=buffer->prev_buffer;
		// 		free(buffer);
		// 		buffer_header->used_buffer--;
		// 	}

		// 	buffer_header->head->frame.number_of_page+=1;
		// 	buffer_header->head->frame.root_page=pagenum;
		// 	buffer_header->head->isDirty=true;
		// 	buffer_header->head->isPinned=false;
		// }
		int res=buf_write_page(table_id,pagenum,src,0);
		return res;
	}
	return -1;
}

void buf_set_root_page_num(int table_id, pagenum_t pagenum){
	buffer_t* buffer=using_buffer(table_id,0,READ);
	if(buffer!=NULL){
		buffer->frame.root_page=pagenum;
		//buffer_header->head->isPinned=false;
		buffer->isDirty=true;
		pthread_mutex_unlock(&(buffer->page_mutex));
	}
}

pagenum_t buf_get_root_page_num(int table_id, int trx_id){
	//printf("read trx:%d page:%ld\n", trx_id,0);
	buffer_t* buffer=using_buffer(table_id,0,READ);
	lock_t* lock_res=NULL;
	if(buffer!=NULL){
		pthread_mutex_unlock(&(buffer->page_mutex));
		if(trx_id>0) lock_res=lock_acquire(table_id, 0, trx_id, READ,0);
		if(lock_res==NULL && trx_id!=0){//abort
			//printf("res:NULL in trx:%d\n", trx_id);
			return -2;
		}
		buffer=using_buffer(table_id,0, READ);
		if(buffer!=NULL){
			pagenum_t root_page_num=buffer->frame.root_page;
			pthread_mutex_unlock(&(buffer->page_mutex));
			return root_page_num;
		}
		return 0;	
	}
	return 0;
}

pagenum_t buf_get_number_of_pages(int table_id){
	buffer_t* buffer=using_buffer(table_id,0,READ);
	if(buffer!=NULL){
		pagenum_t num=buffer->frame.number_of_page;
		pthread_mutex_unlock(&(buffer->page_mutex));
		//buffer_header->head->isPinned=false;
		return num;
	}
	return 0;
}

void buf_free_page(int table_id, pagenum_t pagenum){
	//set header
	buffer_t* buffer=using_buffer(table_id,0,READ);
	pthread_mutex_lock(&buffer_manager_mutex);
	if(buffer!=NULL){
		if(buffer->isDirty){
			file_write_page(table_id,0, &(buffer->frame));
			buffer->isDirty=false;
		}
		file_free_page(table_id,pagenum);
		file_read_page(table_id,0,&(buffer->frame));
		pthread_mutex_unlock(&(buffer->page_mutex));
	}
	//buffer_header->head->isPinned=false;

	//remove page if in buffer
	buffer=get_buffer(table_id, pagenum);
	if(buffer!=NULL){
		if(buffer->prev_buffer!=NULL) buffer->prev_buffer->next_buffer=buffer->next_buffer;
		if(buffer->next_buffer!=NULL) buffer->next_buffer->prev_buffer=buffer->prev_buffer;
		if(buffer==buffer_header->head) buffer_header->head=buffer->next_buffer;
		if(buffer==buffer_header->tail) buffer_header->tail=buffer->prev_buffer;
		free(buffer);
		buffer_header->used_buffer--;
	}
	pthread_mutex_unlock(&buffer_manager_mutex);	
}

pagenum_t buf_alloc_page(int table_id){
	pagenum_t page=0;
	buffer_t* buffer=using_buffer(table_id,0,READ);
	if(buffer!=NULL){
		if(buffer->isDirty){
			file_write_page(table_id,0,&(buffer->frame));
			buffer->isDirty=false;
		}
		page=file_alloc_page(table_id);
		file_read_page(table_id,0,&(buffer->frame));
		pthread_mutex_unlock(&(buffer->page_mutex));
	}
	return page;
}

void print_buf(){
	buffer_t* cur=buffer_header->head;
	for(int i=0;i<buffer_header->used_buffer;i++){
		printf("table_id:%d, pagenum:%ld\n",cur->table_id, cur->pagenum);

		cur=cur->next_buffer;
	}
}


int buf_close_table(int table_id){
	pthread_mutex_lock(&buffer_manager_mutex);
	buffer_t* cur=buffer_header->head;
	while(cur!=NULL){
		if(cur->table_id==table_id){
			if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
			if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
			if(cur->isDirty){
				file_write_page(cur->table_id, cur->pagenum, &(cur->frame));
			}
			if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
			if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
			buffer_t* temp=cur;
			cur=cur->next_buffer;
			pthread_mutex_unlock(&(temp->page_mutex));
			free(temp);
		}
		else cur=cur->next_buffer;

	}
	pthread_mutex_unlock(&buffer_manager_mutex);
	return 0;
}

int buf_shutdown_db(){
	pthread_mutex_lock(&buffer_manager_mutex);
	buffer_t* cur=buffer_header->head;
	while(cur!=NULL){
		if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
		if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
		if(cur->isDirty){
			file_write_page(cur->table_id,cur->pagenum,&(cur->frame));
		}
		if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
		if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
		buffer_t* temp=cur;
		cur=cur->next_buffer;
		pthread_mutex_unlock(&(temp->page_mutex));
		free(temp);
	}
	free(buffers);
	free(buffer_header);
	pthread_mutex_unlock(&buffer_manager_mutex);
	return 0;
}

void buf_file_initialize(int table_id){
	pthread_mutex_lock(&buffer_manager_mutex);
	buffer_t* cur=buffer_header->head;
	while(cur!=NULL){
		if(cur->table_id==table_id){
			if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
			if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
			if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
			if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
			buffer_t* temp=cur;
			cur=cur->next_buffer;
			pthread_mutex_unlock(&(temp->page_mutex));
			free(temp);
		}
		else cur=cur->next_buffer;

	}
	file_initialize(table_id);
	buffer_header->used_buffer=0;
	pthread_mutex_unlock(&buffer_manager_mutex);
}

//trx_buffer control
void init_trx_saved_data(trx_saved_data_t* saved_data){
	if(saved_data!=NULL){
		saved_data->trx_id=0;
		saved_data->prev=NULL;
		saved_data->next=NULL;
	}
}

trx_saved_data_t* find_trx_saved_data(buffer_t* buffer, int trx_id){
	trx_saved_data_t* cur=buffer->saved_data_head;
	while(cur!=NULL){
		if(cur->trx_id==trx_id) return cur;
	}	
	return NULL;
}

void insert_trx_saved_data(buffer_t* buffer, trx_saved_data_t* new_data){
	if(buffer->saved_data_head==NULL && buffer->saved_data_tail==NULL){
		buffer->saved_data_head=new_data;
		buffer->saved_data_tail=new_data;
	}
	else{
		buffer->saved_data_tail->next=new_data;
		new_data->prev=buffer->saved_data_tail;
		buffer->saved_data_tail=new_data;
	}
}

void delete_trx_saved_data(buffer_t* buffer, trx_saved_data_t* tar_data){
	if(tar_data->prev!=NULL) tar_data->prev->next=tar_data->next;
	if(tar_data->next!=NULL) tar_data->next->prev=tar_data->prev;
	if(tar_data==buffer->saved_data_head) buffer->saved_data_head=tar_data->next;
	if(tar_data==buffer->saved_data_tail) buffer->saved_data_tail=tar_data->prev;
}

//delete saved data
int buf_commit_done(int table_id, pagenum_t pagenum, int trx_id, int mode){
	buffer_t* buffer=using_buffer(table_id, pagenum, mode);
	if(buffer!=NULL){
		if(mode==WRITE){
			trx_saved_data_t* saved_data=find_trx_saved_data(buffer, trx_id);
			if(saved_data!=NULL){
				delete_trx_saved_data(buffer, saved_data);
			}
		}
		pthread_mutex_unlock(&(buffer->page_mutex));
		return 0;
	}
	return -1;
}


//ROLLBACK
int buf_abort_done(int table_id, pagenum_t pagenum, int trx_id, int mode){
	buffer_t* buffer=using_buffer(table_id, pagenum, mode);
	if(buffer!=NULL){
		if(mode==WRITE){
			trx_saved_data_t* saved_data=find_trx_saved_data(buffer, trx_id);
			if(saved_data!=NULL){
				buffer->frame=saved_data->saved_frame;
				buffer->isDirty=1;
				delete_trx_saved_data(buffer, saved_data);
			}
			
		}
		pthread_mutex_unlock(&(buffer->page_mutex));
		return 0;
	}
	return -1;
}