#include "buffer.h"

buffer_t* buffers=NULL;
buffer_header_t* buffer_header=NULL;

void init_buf(buffer_t* buffer){
	if(buffer!=NULL){
		buffer->table_id=0;
		buffer->pagenum=0;
		buffer->isDirty=false;
		buffer->isPinned=false;
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
			if(!cur->isPinned){
				if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
				if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
				if(cur->isDirty){
					file_write_page(cur->table_id,cur->pagenum,&(cur->frame));
				}
				if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
				if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
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

int using_buffer(int table_id,pagenum_t pagenum,int mode){
	error_check=false;
	buffer_t* buffer=get_buffer(table_id,pagenum);
	if(buffer==NULL){
		int res=make_free_buffer();
		if(res==-1) {
			error_check=true;
			//printf("no more free page %d\n",buffer_header->used_buffer);
			//printf("buffer->head:%p, buffer->tail:%p\n",buffer_header->head, buffer_header->tail);
			return -1;
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
	buffer_header->head->isPinned=true;
	return 1;
}

int buf_read_page(int table_id, pagenum_t pagenum, page_t* dest){
	int res=using_buffer(table_id,pagenum,READ);
	if(res!=-1){
		(*dest)=buffer_header->head->frame;
		buffer_header->head->isPinned=false;
	}
	if(res==-1) error_check=true;
	return res;
}

int buf_write_page(int table_id, pagenum_t pagenum, const page_t* src){
	int res=using_buffer(table_id,pagenum,WRITE);
	if(res!=-1){
		buffer_header->head->frame=(*src);
		buffer_header->head->isDirty=true;
		buffer_header->head->isPinned=false;
	}
	if(res==-1) error_check=true;
	return res;
}

int buf_read_root_page(int table_id, page_t* dest){
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		if(buffer_header->head->frame.number_of_page==1){//no root page
			buffer_header->head->isPinned=false;
			return -1;
		}
		pagenum_t root_page_num=buffer_header->head->frame.root_page;
		buffer_header->head->isPinned=false;
		res=using_buffer(table_id,root_page_num,READ);
		if(res!=-1){
			(*dest)=buffer_header->head->frame;
			buffer_header->head->isPinned=false;
			return 0;
		}
	}
	if(res==-1) error_check=true;
	return res;
}

int buf_write_root_page(int table_id, pagenum_t pagenum, const page_t* src){
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		if(buffer_header->head->frame.number_of_page==1){
			pagenum_t page=0;
			if(buffer_header->head->isDirty){
				file_write_page(table_id,0,&(buffer_header->head->frame));
				buffer_header->head->isDirty=false;
			}
			page=file_alloc_page(table_id);
			file_read_page(table_id,0,&(buffer_header->head->frame));
			buffer_header->head->frame.root_page=page;
			buffer_header->head->isDirty=true;
			buffer_header->head->isPinned=false;
		}
		else if(pagenum!=buffer_header->head->frame.root_page){
			//free root_page
			if(buffer_header->head->isDirty){
				file_write_page(table_id,0, &(buffer_header->head->frame));
				buffer_header->head->isDirty=false;
			}
			file_free_page(table_id,pagenum);
			file_read_page(table_id,0,&(buffer_header->head->frame));

			buffer_t* buffer=get_buffer(table_id, pagenum);
			if(buffer!=NULL){
				if(buffer->prev_buffer!=NULL) buffer->prev_buffer->next_buffer=buffer->next_buffer;
				if(buffer->next_buffer!=NULL) buffer->next_buffer->prev_buffer=buffer->prev_buffer;
				if(buffer==buffer_header->head) buffer_header->head=buffer->next_buffer;
				if(buffer==buffer_header->tail) buffer_header->tail=buffer->prev_buffer;
				free(buffer);
				buffer_header->used_buffer--;
			}

			buffer_header->head->frame.number_of_page+=1;
			buffer_header->head->frame.root_page=pagenum;
			buffer_header->head->isDirty=true;
			buffer_header->head->isPinned=false;
		}
		res=buf_write_page(table_id,pagenum,src);
	}
	return res;
}

void buf_set_root_page_num(int table_id, pagenum_t pagenum){
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		buffer_header->head->frame.root_page=pagenum;
		buffer_header->head->isPinned=false;
		buffer_header->head->isDirty=true;
	}
}

pagenum_t buf_get_root_page_num(int table_id){
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		pagenum_t root_page_num=buffer_header->head->frame.root_page;
		buffer_header->head->isPinned=false;
		return root_page_num;
	}
	return 0;
}

pagenum_t buf_get_number_of_pages(int table_id){
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		pagenum_t num=buffer_header->head->frame.number_of_page;
		buffer_header->head->isPinned=false;
		return num;
	}
	return 0;
}

void buf_free_page(int table_id, pagenum_t pagenum){
	//set header
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		if(buffer_header->head->isDirty){
			file_write_page(table_id,0, &(buffer_header->head->frame));
			buffer_header->head->isDirty=false;
		}
		file_free_page(table_id,pagenum);
		file_read_page(table_id,0,&(buffer_header->head->frame));
	}
	buffer_header->head->isPinned=false;
	//remove page if in buffer
	buffer_t* buffer=get_buffer(table_id, pagenum);
	if(buffer!=NULL){
		if(buffer->prev_buffer!=NULL) buffer->prev_buffer->next_buffer=buffer->next_buffer;
		if(buffer->next_buffer!=NULL) buffer->next_buffer->prev_buffer=buffer->prev_buffer;
		if(buffer==buffer_header->head) buffer_header->head=buffer->next_buffer;
		if(buffer==buffer_header->tail) buffer_header->tail=buffer->prev_buffer;
		free(buffer);
		buffer_header->used_buffer--;
	}
}

pagenum_t buf_alloc_page(int table_id){
	pagenum_t page=0;
	int res=using_buffer(table_id,0,READ);
	if(res!=-1){
		if(buffer_header->head->isDirty){
			file_write_page(table_id,0,&(buffer_header->head->frame));
			buffer_header->head->isDirty=false;
		}
		page=file_alloc_page(table_id);
		file_read_page(table_id,0,&(buffer_header->head->frame));
	}
	buffer_header->head->isPinned=false;
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
			free(temp);
		}
		else cur=cur->next_buffer;

	}
	return 0;
}

int buf_shutdown_db(){
	buffer_t* cur=buffer_header->head;
	while(cur!=NULL){
		if(!cur->isPinned){
			if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
			if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
			if(cur->isDirty){
				file_write_page(cur->table_id,cur->pagenum,&(cur->frame));
			}
			if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
			if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
			buffer_t* temp=cur;
			cur=cur->next_buffer;
			free(temp);
		}else return -1;
	}
	free(buffers);
	free(buffer_header);
	return 0;
}

void buf_file_initialize(int table_id){
	buffer_t* cur=buffer_header->head;
	while(cur!=NULL){
		if(cur->table_id==table_id){
			if(cur->prev_buffer!=NULL) cur->prev_buffer->next_buffer=cur->next_buffer;
			if(cur->next_buffer!=NULL) cur->next_buffer->prev_buffer=cur->prev_buffer;
			if(cur==buffer_header->head) buffer_header->head=cur->next_buffer;
			if(cur==buffer_header->tail) buffer_header->tail=cur->prev_buffer;
			buffer_t* temp=cur;
			cur=cur->next_buffer;
			free(temp);
		}
		else cur=cur->next_buffer;

	}
	file_initialize(table_id);
	buffer_header->used_buffer=0;
}