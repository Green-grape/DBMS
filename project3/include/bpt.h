#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#define MAX_TABLE_ID 10
#define int_padding 104
#define MAX_KEY_INTL 248
#define MAX_KEY_LEAF 31
#define true 1
#define false 0
#define VALUE_PAIR_SIZE 128

#pragma pack(1)

//struct
typedef struct value_pair{
    int64_t key;
    char value[120];
}value_pair;

typedef struct page_pair{
    int64_t key;
    pagenum_t page_num;
}page_pair;

typedef struct internal_page_t{
    pagenum_t parent_page_num;
    int isLeaf;
    int number_of_keys;
    char padding[int_padding];
    pagenum_t left_most_page_num;
    page_pair key_page[MAX_KEY_INTL];
}internal_page_t;

typedef struct leaf_page_t{
    pagenum_t parent_page_num;
    int isLeaf;
    int number_of_keys;
    char padding[int_padding];
    pagenum_t right_sibling_page_num;
    value_pair key_value[MAX_KEY_LEAF];
}leaf_page_t;

//GLOBAL
extern char filepaths[11][21];

extern int filepathNum;

extern int isOpen[MAX_TABLE_ID+1];

extern page_t* header_pages[MAX_TABLE_ID+1];

//Util
void init_leaf(leaf_page_t* leaf_page);
void init_internal(internal_page_t* internal_page);
int cut( int length );
int db_get_left_index(int table_id, pagenum_t parent_page_num, pagenum_t left_page_num) ;
void print_internal(internal_page_t*);
void print_leaf(leaf_page_t*);
void print_page();
void usage();

//open
int open_table(char* pathname);
int init_db(int num_buf);
int close_table(int table_id);
int shutdown_db();

//find
int64_t db_find_leaf(int table_id, int64_t key);
int db_find(int table_id, int64_t key, char* ret_val);

//Insert
int db_insert_into_leaf(int table_id, int64_t leaf_page_num, leaf_page_t* leaf_page, int64_t key, char *value);
int db_insert_into_leaf_after_splitting(int table_id, int64_t leaf_page_num, leaf_page_t* leaf_page, int64_t key, char* value);
int db_insert_into_internal(int table_id,pagenum_t parent_page_num,pagenum_t left_page_num, pagenum_t right_page_num, int64_t key, int left_index, internal_page_t* parent_page);
int db_insert_into_internal_after_splitting(int table_id,pagenum_t old_page_num, pagenum_t right_page_num, int64_t key, int left_index, internal_page_t* old_page);
int db_insert_into_parent(int table_id, pagenum_t parent_page_num,pagenum_t left_page_num,pagenum_t right_page_num,int64_t key);
int db_insert_into_new_root(int table_id, pagenum_t left_page_num, pagenum_t right_page_num, int64_t key);
int db_start_new_tree(int table_id, int64_t key, char* value);
int db_set_right_sibling_insert(int table_id, int64_t parent_page_num, int64_t child_page_num);
int db_insert(int table_id, int64_t key, char* value);

//Deletion

leaf_page_t* db_remove_entry_from_leaf(int table_id, int64_t leaf_page_num, int64_t key);
int db_adjust_root(int table_id, leaf_page_t* leaf_page,int64_t leaf_page_num);
int db_delete_entry(int table_id, int64_t leaf_page_num,int64_t key);
int db_delayed_merge_internal(int table_id, int64_t parent_page_num, int64_t child_page_num);
int db_set_right_sibling_delete(int table_id, int64_t parent_page_num, int64_t child_page_num);
int db_delete(int table_id, int64_t key);

#endif /* __BPT_H__*/
