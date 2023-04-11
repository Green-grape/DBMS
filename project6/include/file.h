#ifndef __FILE_H_
#define __FILE_H_

//#define _GNU_SOURCE
#define page_size 4096
#define heap_padding 4072
#define true 1
#define false 0

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <malloc.h>
#include <string.h>

#pragma pack(1)

extern char filepaths[11][21];
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
pagenum_t file_alloc_page(int table_id);
void file_free_page(int table_id, pagenum_t pagenum);
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest);
void file_write_page(int table_id, pagenum_t pagenum, const page_t* src);
void file_initialize(int table_id);

#endif 
//__FILE_H_
