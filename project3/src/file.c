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

pagenum_t file_alloc_page(int table_id) {
	page_t *header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	int64_t res=0;
	file_read_page(table_id,0, header_page);
	if(error_check==false){
		if (header_page->next_free_page == 0) { //no more free page
			header_page->number_of_page+=1;
			file_write_page(table_id, 0, header_page);
			if(error_check==false) res=header_page->number_of_page-1;
		}
		else {
			page_t* next_page=(page_t*)malloc(sizeof(page_t));
			memset(next_page,0,sizeof(page_t));
			file_read_page(table_id, header_page->next_free_page, next_page);
			if(error_check==false){
				res=header_page->next_free_page;
				header_page->next_free_page = next_page->next_free_page;
				header_page->number_of_page+=1;
				file_write_page(table_id, 0, header_page);
				if(error_check) res=0; 
			}
			free(next_page);
		}
	}
	if(header_page!=NULL)free(header_page);
	return res;
}

void file_free_page(int table_id, pagenum_t pagenum) {
	page_t* page=(page_t*)malloc(sizeof(page_t));
	memset(page,0,sizeof(page_t));
	pagenum_t cur_page;

	file_read_page(table_id, 0, page); //start from header page
	cur_page = 0;
	page->number_of_page--;
	file_write_page(table_id, 0,page);

	if(error_check==false){
		while (1) {
			uint64_t next_page = page->next_free_page;
			if (next_page > pagenum || next_page==0) {
				page->next_free_page = pagenum;
				file_write_page(table_id, cur_page, page); //set current_page
				if(error_check==true) {
					printf("file_free_page,set cur page");
					break;
				}
				page->next_free_page = next_page;
				file_write_page(table_id, pagenum, page); //put new free page
				if(error_check==true){
					printf("file_free_page, put new page");
				}
				break;
			}
			else{
				file_read_page(table_id, next_page,page);
				if(error_check==true) break;
			}
			cur_page = next_page;
		}
	}
	else printf("file_free_page read\n");
	if(page!=NULL) free(page);
}

void file_read_page(int table_id, pagenum_t pagenum,page_t* dest) {
	error_check=false;
	eof_check=false;
	int fd = open(filepaths[table_id], O_RDWR | O_SYNC, 0666);
	if (fd == -1) {
		fd = open(filepaths[table_id], O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0666);
	}
	if(fd!=-1) {
		if (page_size*pagenum==lseek(fd, page_size*pagenum, SEEK_SET)) {
			ssize_t read_count=read(fd,dest,page_size);
			if (read_count==-1) { //error
				printf("file_read_page,read\n");
				error_check=true;
			}
			else if(read_count<page_size) {
				eof_check=true;
				if(pagenum==0){//no header
					page_t* header_page=(page_t*)malloc(sizeof(page_t));
					memset(header_page,0,sizeof(page_t));
					init_header(header_page);
					file_write_page(table_id,0,header_page);
					(*dest)=(*header_page);
					free(header_page);
				}
			} //reach to EOF
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

void file_write_page(int table_id, pagenum_t pagenum, const page_t* src) {
	error_check=false;
	int fd = open(filepaths[table_id], O_RDWR | O_SYNC, 0666);
	if (fd == -1) {
		fd = open(filepaths[table_id], O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0666);
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
			perror("file_write_page,lseek");
			error_check=true;
		}
		close(fd);
	}
	else{
		printf("file_write_page,open\n");
		error_check=true;
	}
}

void file_initialize(int table_id){
	page_t* header_page=(page_t*)malloc(sizeof(page_t));
	memset(header_page,0,sizeof(page_t));
	file_read_page(table_id,0,header_page);
	if(error_check==false){
		init_header(header_page);
		file_write_page(table_id,0,header_page);
	}
	free(header_page);
}

