#ifndef __FILE_H_
#define __FILE_H_

#define _GNU_SOURCE
#define page_size 4096
#define heap_padding 4072
#define true 1
#define false 0

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <io.h> needed for visual studio
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <malloc.h>
#include <string.h>

#pragma pack(1)

extern char filepath[100];
extern int eof_check; //if EOF 1,else 0
extern int error_check; //if error is happened in file.c set 1, else 0

typedef uint64_t pagenum_t;
typedef struct page_t { //using header page to cover header and free page in disk operations
	uint64_t next_free_page;
	uint64_t root_page;
	uint64_t number_of_page;
	char padding[heap_padding];
}page_t;

void init_header(page_t* header_page);
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
int file_is_in_free_list(pagenum_t pagenum);
void file_read_page(pagenum_t pagenum, page_t* dest);
void file_read_root_page(page_t** dest);
void file_write_root_page(pagenum_t pagenum, const page_t*src);
void file_write_page(pagenum_t pagenum, const page_t* src);
pagenum_t file_get_root_page_num();
void file_set_root_page_num(pagenum_t pagenum);
void file_initialize();
int64_t file_get_number_of_pages();
int file_get_table_id();

#endif 
//__FILE_H_