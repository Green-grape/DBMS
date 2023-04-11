#include "file.h"

//GLOBAL
int eof_check=0;
int error_check=0;

void init_header(page_t* header_page){
	if(header_page!=NULL){
		header_page->next_free_page=0;
		header_page->root_page=1;
		header_page->number_of_page=1;
	}
}

pagenum_t file_alloc_page() {
	page_t *header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	int64_t res=0;
	file_read_page(0, header_page);
	if(error_check==false){
		if (header_page->next_free_page == 0) { //no more free page
			header_page->number_of_page+=1;
			file_write_page(0, header_page);
			if(error_check==false) res=header_page->number_of_page-1;
		}
		else {
			page_t* next_page=(page_t*)malloc(sizeof(page_t));
			memset(next_page,0,sizeof(page_t));
			file_read_page(header_page->next_free_page, next_page);
			if(error_check==false){
				res=header_page->next_free_page;
				header_page->next_free_page = next_page->next_free_page;
				header_page->number_of_page+=1;
				file_write_page(0, header_page);
				if(error_check) res=0; 
			}
			free(next_page);
		}
	}
	if(header_page!=NULL)free(header_page);
	return res;
}

void file_free_page(pagenum_t pagenum) {
	page_t* page=(page_t*)malloc(sizeof(page_t));
	memset(page,0,sizeof(page_t));
	pagenum_t cur_page;

	file_read_page(0, page); //start from header page
	cur_page = 0;
	page->number_of_page--;
	file_write_page(0,page);

	if(error_check==false){
		while (1) {
			uint64_t next_page = page->next_free_page;
			if (next_page > pagenum || next_page==0) {
				page->next_free_page = pagenum;
				file_write_page(cur_page, page); //set current_page
				if(error_check==true) {
					printf("file_free_page,set cur page");
					break;
				}
				page->next_free_page = next_page;
				file_write_page(pagenum, page); //put new free page
				if(error_check==true){
					printf("file_free_page, put new page");
				}
				break;
			}
			else{
				file_read_page(next_page,page);
				if(error_check==true) break;
			}
			cur_page = next_page;
		}
	}
	else printf("file_free_page read\n");
	if(page!=NULL) free(page);
}

int file_is_in_free_list(pagenum_t pagenum){
	page_t *page=(page_t*)malloc(sizeof(page_t));
	memset(page,0,sizeof(page_t));
	pagenum_t cur_page;

	int res;
	file_read_page(0,page);
	cur_page=0;
	if(error_check==false){
		while(1){
			pagenum_t next_page=page->next_free_page;
			if(next_page==0) {
				res=false;
				break;
			}
			if(cur_page==pagenum) {
				res=true;
				break;
			}
			file_read_page(next_page,page);
			if(error_check==true){
				break;
				res=-1;
			}
			cur_page=next_page;
		}
	}
	else {
		printf("file_is_in_free_list,read_page");
		res=false;
	}
	if(page!=NULL) free(page);
	return res;
}

void file_read_page(pagenum_t pagenum,page_t* dest) {
	error_check=false;
	eof_check=false;
	int fd = open(filepath, O_RDWR | O_SYNC, 0666);
	if (fd == -1) {
		fd = open(filepath, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0666);
	}
	if(fd!=-1) {
		if (page_size*pagenum==lseek(fd, page_size*pagenum, SEEK_SET)) {
			ssize_t read_count=read(fd,dest,page_size);
			if (read_count==-1) { //error
				printf("file_read_page,read\n");
				error_check=true;
			}
			else if(read_count<page_size) eof_check=true; //reach to EOF
			else {
				eof_check=false;
			}
		}
		else {
			printf("file_read_page,lseek\n");
			error_check=true;
		}
		close(fd);
	}
	else {
		printf("file_read_page,open\n");
		error_check=true;
	}
}

void file_write_page(pagenum_t pagenum, const page_t* src) {
	error_check=false;
	int fd = open(filepath, O_RDWR | O_SYNC, 0666);
	if (fd == -1) {
		fd = open(filepath, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0666);
	}
	if(fd!=-1) {
		if (page_size*pagenum == lseek(fd, page_size*pagenum, SEEK_SET)) {
			ssize_t write_count=write(fd,src,page_size);
			if (write_count == -1) { //error
				printf("file_write_page,write\n");
				perror("file_write_page");
				error_check=true;
			}
		}
		else{
			printf("file_write_page,lseek\n");
			error_check=true;
		}
		close(fd);
	}
	else{
		printf("file_write_page,open\n");
		error_check=true;
	}
}

void file_read_root_page(page_t** dest){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(0,header_page);
	if(error_check==false){
		if(eof_check==true){ //no header_page
			init_header(header_page);
			file_write_page(0,header_page);
			if(error_check==true) printf("file_read_root_page,make header\n");
			if((*dest)!=NULL) free(*dest);
			(*dest)=NULL;
		} 
		else{
			if(header_page->number_of_page==1){ //no root_page
				if((*dest)!=NULL) free(*dest);
				(*dest)=NULL;
			}
			else{
				file_read_page(header_page->root_page,*dest);
				if(error_check==true) printf("file_read_root_page,read_root");
			}
		}
	}
	else{
		printf("file_read_root_page,read header\n");
	}
	if(header_page!=NULL) free(header_page);
}

void file_write_root_page(pagenum_t root_page_num, const page_t* src){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(0,header_page);
	if(error_check==false){
		if(eof_check==true){ //no header_page
			init_header(header_page);
		}
		if(root_page_num!=header_page->root_page) {
			file_free_page(header_page->root_page);
			file_read_page(0,header_page);
			header_page->number_of_page+=1;
			header_page->root_page=root_page_num;
		}
		if(header_page->number_of_page==1){//no root_page
			pagenum_t root_page_num=file_alloc_page();
			file_read_page(0,header_page);
			header_page->root_page=root_page_num;
		}
		file_write_page(0,header_page);
		if(error_check==true) {
			printf("file_write_root_page,write header");
		}
		else{
			file_write_page(root_page_num,src);
			if(error_check==true) printf("file_write_root_page,write root\n");
		}
	}
	else printf("file_write_root_page,open\n");
	if(header_page!=NULL) free(header_page);
}

pagenum_t file_get_root_page_num(){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(0, header_page);
	int64_t res;
	if(error_check==false){
		if(eof_check==true) res=0;
		else{
			res=header_page->root_page;
		}
	}
	else {
		printf("file_get_root_page_num\n");
		res=0;
	}
	if(header_page!=NULL) free(header_page);
	return res;
}

void file_set_root_page_num(pagenum_t pagenum){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(0, header_page);
	if(error_check==false){
		header_page->root_page=pagenum;
	}
	else {
		printf("file_get_root_page_num\n");
	}
	file_write_page(0,header_page);
	if(header_page!=NULL) free(header_page);
}

void file_initialize(){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(0,header_page);
	if(error_check==false){
		init_header(header_page);
		file_write_page(0,header_page);
	}
	free(header_page);
}

int file_get_table_id(){
	int fd = open("table_id.txt", O_RDWR | O_SYNC, 0666);
	if(fd==-1){
		fd = open("table_id.txt", O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0666);
	}
	int table_id=1;
	int res=-1;
	if(fd!=-1){
		if(0==lseek(fd,0,SEEK_SET)){
			ssize_t read_count=read(fd,&table_id,sizeof(table_id));
			if(read_count==-1){
				printf("file_get_table_id,read\n");
			}
			else{
				table_id++;
				if(0==lseek(fd,0,SEEK_SET)){
					if(write(fd,&table_id,sizeof(table_id))==-1){
						printf("file_get_table_id,write\n");
						res=-1;
					}
					else{
						res=table_id-1;
					}
				}	
			}
		}
	}
	else {
		printf("file_get_table_id,open\n");
	}
	if(fd!=-1) close(fd);
	return res;
}

int64_t file_get_number_of_pages(){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(0,header_page);
	int64_t res=header_page->number_of_page;
	free(header_page);
	return res;
}

