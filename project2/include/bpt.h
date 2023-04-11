#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
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
extern char filepath[100];

extern int isOpen;

extern internal_page_t* root_page;//root_page normally internal

//Util
void init_leaf(leaf_page_t* leaf_page);
void init_internal(internal_page_t* internal_page);
int cut( int length );
int db_get_left_index(pagenum_t parent_page_num, pagenum_t left_page_num) ;
void print_internal(internal_page_t*);
void print_leaf(leaf_page_t*);
void print_page();
void usage();

//open
int open_table(char* pathname);

//find
int64_t db_find_leaf(int64_t key);
int db_find(int64_t key, char* ret_val);

//Insert
int db_insert_into_leaf(int64_t leaf_page_num, leaf_page_t* leaf_page, int64_t key, char *value);
int db_insert_into_leaf_after_splitting(int64_t leaf_page_num, leaf_page_t* leaf_page, int64_t key, char* value);
int db_insert_into_internal(pagenum_t parent_page_num,pagenum_t left_page_num, pagenum_t right_page_num, int64_t key, int left_index, internal_page_t* parent_page);
int db_insert_into_internal_after_splitting(pagenum_t old_page_num, pagenum_t right_page_num, int64_t key, int left_index, internal_page_t* old_page);
int db_insert_into_parent(pagenum_t parent_page_num,pagenum_t left_page_num,pagenum_t right_page_num,int64_t key);
int db_insert_into_new_root(pagenum_t left_page_num, pagenum_t right_page_num, int64_t key);
int db_start_new_tree(int64_t key, char* value);
int db_set_right_sibling_insert(int64_t parent_page_num, int64_t child_page_num);
int db_insert(int64_t key, char* value);

//Deletion

leaf_page_t* db_remove_entry_from_leaf(int64_t leaf_page_num, int64_t key);
int db_adjust_root(leaf_page_t* leaf_page,int64_t leaf_page_num);
int db_delete_entry(int64_t leaf_page_num,int64_t key);
int db_delayed_merge_internal(int64_t parent_page_num, int64_t child_page_num);
int db_set_right_sibling_delete(int64_t parent_page_num, int64_t child_page_num);
int db_delete(int64_t key);

#endif /* __BPT_H__*/
