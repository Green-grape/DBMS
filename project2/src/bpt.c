/*
 *  bpt.c  
 */
#include "bpt.h"


//GLOBAL
char filepath[100];

int isOpen=false;

internal_page_t* root_page;//root_page normally internal

//Utility

void usage(){
    printf("1. o file_path: open_table with file path\n2. i key value: insert value with key \n3. f key value: find value with key \n4. d key: delete value with key\n" );
}

void init_leaf(leaf_page_t* leaf_page){
	if(leaf_page!=NULL){
		leaf_page->parent_page_num=0;
		leaf_page->isLeaf=true;
		leaf_page->number_of_keys=0;
		leaf_page->right_sibling_page_num=0;
	}
}

void init_internal(internal_page_t* internal_page){
	if(internal_page!=NULL){
		internal_page->parent_page_num=0;
		internal_page->isLeaf=false;
		internal_page->number_of_keys=0;
	}
}
//open

int open_table(char* pathname){
	strcpy(filepath,pathname);
	if(root_page!=NULL){
		free(root_page);
		root_page=NULL;
	}
	(root_page)=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(root_page,0,sizeof(internal_page_t));
	file_read_root_page(((page_t**)&root_page));
	if(error_check==false){
		isOpen=true;
		return file_get_table_id();
	}
	return -1;
}

//find

int64_t db_find_leaf(int64_t key) {
    int i = 0;

    internal_page_t* cur_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(cur_page,0,sizeof(internal_page_t));
    file_read_root_page((page_t**)&cur_page);

    if(cur_page==NULL) return -1;

    int64_t res=file_get_root_page_num();

    while (!cur_page->isLeaf) {
        i = 0;
        while (i < cur_page->number_of_keys) {
            if (key >= cur_page->key_page[i].key) i++;
            else break;
        }
        i-=1;
       if(i==-1){
       		res=cur_page->left_most_page_num;
       		file_read_page(res,(page_t*)cur_page);
       		if(error_check==true){
       			printf("find_leaf\n");
       			res=-1;
       			break;
       		}
       }
       else{
       		res=cur_page->key_page[i].page_num;
       		file_read_page(res,(page_t*)cur_page);
       		if(error_check==true){
       			printf("find_leaf\n");
       			res=-1;
       			break;
       		}
       }
       printf("recursion\n");
    }

    free(cur_page);
    cur_page=NULL;
    return res;
}


/* Finds and returns the record to which
 * a key refers.
 */
int db_find(int64_t key, char* ret_val) {
    int i = 0;
    int res=-1;

    int64_t leaf_page_num =db_find_leaf(key);
    if(leaf_page_num < 1) {
        printf("no leaf page");
        return -1;
    }
    // for (i = 0; i < c->num_keys; i++)
    //     if (c->keys[i] == key) break;
    // if (i == c->num_keys) 
    //     return NULL;
    // else
    //     return (record *)c->pointers[i];
    leaf_page_t* leaf_page=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    memset(leaf_page,0,sizeof(leaf_page_t));
    file_read_page(leaf_page_num,(page_t*)leaf_page);
    if(error_check==false){
    	for(i=0;i<leaf_page->number_of_keys;i++){
    		if(leaf_page->key_value[i].key==key) break;
    	}
    	if(i==leaf_page->number_of_keys) {
            printf("no key");
            res=-1;
        }
    	else {
    		strcpy(ret_val,leaf_page->key_value[i].value);
    		res=0;
    	}
    }
    else res=-1;
    free(leaf_page);
    return res;

}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


int db_get_left_index(pagenum_t parent_page_num, pagenum_t left_page_num) {

	internal_page_t* parent_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(parent_page,0,sizeof(internal_page_t));
	file_read_page(parent_page_num,(page_t*)parent_page);
	int res;
	if(error_check==false){
		int left_index = 0;
		if(parent_page->left_most_page_num==left_page_num) res=-1;
    	else{
    		while (left_index < parent_page->number_of_keys && parent_page->key_page[left_index].page_num != left_page_num){
                left_index++;
            }
    		res=left_index;
    	}
	}else{
		printf("get left index\n");
		res=-2;
	}
	free(parent_page);
	return res;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int db_insert_into_leaf(int64_t leaf_page_num, leaf_page_t* leaf_page, int64_t key, char *value) {

    int i, insertion_point;

    insertion_point = 0;
    // while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] < key)
    //     insertion_point++;

    // for (i = leaf->num_keys; i > insertion_point; i--) {
    //     leaf->keys[i] = leaf->keys[i - 1];
    //     leaf->pointers[i] = leaf->pointers[i - 1];
    // }
    // leaf->keys[insertion_point] = key;
    // leaf->pointers[insertion_point] = pointer;
    // leaf->num_keys++;
    // return leaf;
    while(insertion_point< leaf_page->number_of_keys && leaf_page->key_value[insertion_point].key<key){
    	insertion_point++;
    }

    for(i=leaf_page->number_of_keys; i>insertion_point;i--){
    	leaf_page->key_value[i]=leaf_page->key_value[i-1];
    }

    leaf_page->key_value[insertion_point].key=key;
    strcpy(leaf_page->key_value[insertion_point].value,value);
    leaf_page->number_of_keys++;
    file_write_page(leaf_page_num,(page_t*)leaf_page);
    if(error_check==false) return 0;
    else return -1;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
int db_insert_into_leaf_after_splitting(int64_t leaf_page_num, leaf_page_t* leaf_page, int64_t key, char* value) {

    // node * new_leaf;
    // int * temp_keys;
    // void ** temp_pointers;
    // int insertion_index, split, new_key, i, j;

    // new_leaf = make_leaf();

    // temp_keys = malloc( order * sizeof(int) );
    // if (temp_keys == NULL) {
    //     perror("Temporary keys array.");
    //     exit(EXIT_FAILURE);
    // }

    // temp_pointers = malloc( order * sizeof(void *) );
    // if (temp_pointers == NULL) {
    //     perror("Temporary pointers array.");
    //     exit(EXIT_FAILURE);
    // }
    value_pair temp[MAX_KEY_LEAF+1];
    int insertion_index,split,i,j;
  
    leaf_page_t* new_leaf_page=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    memset(new_leaf_page,0,sizeof(leaf_page_t));
    init_leaf(new_leaf_page); 
    print_leaf(leaf_page);

    // insertion_index = 0;
    // while (insertion_index < order - 1 && leaf->keys[insertion_index] < key)
    //     insertion_index++;

    // for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
    //     if (j == insertion_index) j++;
    //     temp_keys[j] = leaf->keys[i];
    //     temp_pointers[j] = leaf->pointers[i];
    // }

    // temp_keys[insertion_index] = key;
    // temp_pointers[insertion_index] = pointer;

    // leaf->num_keys = 0;

    // split = cut(order - 1);

    // for (i = 0; i < split; i++) {
    //     leaf->pointers[i] = temp_pointers[i];
    //     leaf->keys[i] = temp_keys[i];
    //     leaf->num_keys++;
    // }

    // for (i = split, j = 0; i < order; i++, j++) {
    //     new_leaf->pointers[j] = temp_pointers[i];
    //     new_leaf->keys[j] = temp_keys[i];
    //     new_leaf->num_keys++;
    // }
    insertion_index=0;
    while(insertion_index<leaf_page->number_of_keys && leaf_page->key_value[insertion_index].key<key){
        insertion_index++;
    }
    
    for(i=0,j=0;i<leaf_page->number_of_keys;i++,j++){
    	if(j==insertion_index) j++;
    	temp[j].key=leaf_page->key_value[i].key;
        strncpy(temp[j].value,leaf_page->key_value[i].value,strlen(leaf_page->key_value[i].value)+1);
    }
    temp[insertion_index].key=key;
    strncpy(temp[insertion_index].value,value,strlen(value)+1);  

    split=cut(MAX_KEY_LEAF);

    leaf_page->number_of_keys=0;
    for(i=0;i<split;i++){
    	leaf_page->key_value[i].key=temp[i].key;
        strncpy(leaf_page->key_value[i].value,temp[i].value,strlen(temp[i].value)+1);
    	leaf_page->number_of_keys++;
    }
    print_leaf(leaf_page);

    for(i=split,j=0;i<MAX_KEY_LEAF+1;i++,j++){
        new_leaf_page->key_value[j].key=temp[i].key;
        strncpy(new_leaf_page->key_value[j].value,temp[i].value,strlen(temp[i].value)+1);
        new_leaf_page->number_of_keys++;
    }
    print_leaf(new_leaf_page);
    

    // new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    // leaf->pointers[order - 1] = new_leaf;

    // for (i = leaf->num_keys; i < order - 1; i++)
    //     leaf->pointers[i] = NULL;
    // for (i = new_leaf->num_keys; i < order - 1; i++)
    //     new_leaf->pointers[i] = NULL;

    // new_leaf->parent = leaf->parent;
    // new_key = new_leaf->keys[0];

    new_leaf_page->parent_page_num=leaf_page->parent_page_num;
    int64_t new_key=new_leaf_page->key_value[0].key;
    pagenum_t parent_page_num=leaf_page->parent_page_num;
    new_leaf_page->right_sibling_page_num=leaf_page->right_sibling_page_num;
    pagenum_t new_leaf_page_num=file_alloc_page();
    leaf_page->right_sibling_page_num=new_leaf_page_num;
    //buffer need modify
    if(new_leaf_page_num>0){
    	file_write_page(leaf_page_num,(page_t*)leaf_page);
    	if(error_check==false){
    		file_write_page(new_leaf_page_num,(page_t*)new_leaf_page);
    		free(new_leaf_page);
            new_leaf_page=NULL;
    		if(error_check==false) {
                return db_insert_into_parent(parent_page_num,leaf_page_num,new_leaf_page_num,new_key);
            }
            else  printf("db_insert_into_leaf_after_splitting,write new leaf\n");
    	}
        else{
            printf("db_insert_into_leaf_after_splitting,write leaf\n");
        }
    }
    else{
    	printf("db_insert_into_leaf_after_splitting,alloc page\n");
    }
    if(new_leaf_page!=NULL) free(new_leaf_page);
    return -1;
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
int db_insert_into_internal(pagenum_t parent_page_num,pagenum_t left_page_num, pagenum_t right_page_num, int64_t key, int left_index, internal_page_t* parent_page) {
    int i;

    // for (i = n->num_keys; i > left_index; i--) {
    //     n->pointers[i + 1] = n->pointers[i];
    //     n->keys[i] = n->keys[i - 1];
    // }
    // n->pointers[left_index + 1] = right;
    // n->keys[left_index] = key;
    // n->num_keys++;
    for(i=parent_page->number_of_keys-1;i>left_index;i--){
    	parent_page->key_page[i+1]=parent_page->key_page[i];
    }
    parent_page->key_page[left_index+1].page_num=right_page_num;
    parent_page->key_page[left_index+1].key=key;
    parent_page->number_of_keys++;
    file_write_page(parent_page_num,(page_t*)parent_page);
    if(error_check==false){
        internal_page_t* right_page=(internal_page_t*)malloc(sizeof(internal_page_t));
        memset(right_page,0,sizeof(internal_page_t));
        file_read_page(right_page_num,(page_t*)right_page);
        if(right_page->isLeaf){
            db_set_right_sibling_insert(parent_page_num,right_page_num);
        }
        free(right_page);
        right_page=NULL;
    	return 0;
    }
    return -1;
    
    // return root;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
int db_insert_into_internal_after_splitting(pagenum_t old_page_num, pagenum_t right_page_num, int64_t key, int left_index, internal_page_t* old_page) {

    int i, j, split;
    int64_t k_prime;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    // temp_pointers = malloc( (order + 1) * sizeof(node *) );
    // if (temp_pointers == NULL) {
    //     perror("Temporary pointers array for splitting nodes.");
    //     exit(EXIT_FAILURE);
    // }
    // temp_keys = malloc( order * sizeof(int) );
    // if (temp_keys == NULL) {
    //     perror("Temporary keys array for splitting nodes.");
    //     exit(EXIT_FAILURE);
    // }
    page_pair temp_pair[MAX_KEY_INTL+1];

    // for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
    //     if (j == left_index + 1) j++;
    //     temp_pointers[j] = old_node->pointers[i];
    // }

    // for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
    //     if (j == left_index) j++;
    //     temp_keys[j] = old_node->keys[i];
    // }
    // temp_pointers[left_index + 1] = right;
    // temp_keys[left_index] = key;

    for(i=0,j=0;i<old_page->number_of_keys;i++,j++){
    	if(j==left_index+1) j++;
    	temp_pair[j]=old_page->key_page[i];
    }
    temp_pair[left_index+1].page_num=right_page_num;
    temp_pair[left_index+1].key=key;
   

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    // split = cut(order);
    // new_node = make_node();
    // old_node->num_keys = 0;
    // for (i = 0; i < split - 1; i++) {
    //     old_node->pointers[i] = temp_pointers[i];
    //     old_node->keys[i] = temp_keys[i];
    //     old_node->num_keys++;
    // }
    // old_node->pointers[i] = temp_pointers[i];
    // k_prime = temp_keys[split - 1];
    // for (++i, j = 0; i < order; i++, j++) {
    //     new_node->pointers[j] = temp_pointers[i];
    //     new_node->keys[j] = temp_keys[i];
    //     new_node->num_keys++;
    // }
    // new_node->pointers[j] = temp_pointers[i];
    // free(temp_pointers);
    // free(temp_keys);
    // new_node->parent = old_node->parent;
    // for (i = 0; i <= new_node->num_keys; i++) {
    //     child = new_node->pointers[i];
    //     child->parent = new_node;
    // }
    split=cut(MAX_KEY_INTL);
    internal_page_t* new_internal=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(new_internal,0,sizeof(internal_page_t));
    init_internal(new_internal);

    internal_page_t* child_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(child_page, 0, sizeof(internal_page_t));

    old_page->number_of_keys=0;
    for(i=0;i<split-1;i++){
    	old_page->key_page[i]=temp_pair[i];
    	old_page->number_of_keys++;
        if(i==split-2){
            internal_page_t* t_internal=(internal_page_t*)malloc(sizeof(internal_page_t));
            memset(t_internal,0,sizeof(internal_page_t));
            file_read_page(temp_pair[i].page_num, (page_t*)t_internal);
            if(t_internal->isLeaf){
                ((leaf_page_t*)t_internal)->right_sibling_page_num=0;
                file_write_page(temp_pair[i].page_num,(page_t*)t_internal);
            }
            free(t_internal);
        }
    }
    if(left_index+1<split-1){
        file_read_page(right_page_num,(page_t*)child_page);
        child_page->parent_page_num=old_page_num;
        file_write_page(right_page_num,(page_t*)child_page);
    }
    k_prime=temp_pair[split-1].key;
    new_internal->left_most_page_num=temp_pair[split-1].page_num;

    pagenum_t new_internal_page_num=file_alloc_page();
    file_read_page(new_internal->left_most_page_num, (page_t*)child_page);
    child_page->parent_page_num=new_internal_page_num;
    file_write_page(new_internal->left_most_page_num,(page_t*)child_page);
    for(++i,j=0;i<=MAX_KEY_INTL;i++,j++){
    	new_internal->key_page[j]=temp_pair[i];
    	new_internal->number_of_keys++;
        file_read_page(new_internal->key_page[j].page_num, (page_t*)child_page);
        child_page->parent_page_num=new_internal_page_num;
        file_write_page(new_internal->key_page[j].page_num, (page_t*)child_page);
    }
    free(child_page);
    new_internal->parent_page_num=old_page->parent_page_num;

    pagenum_t parent_page_num=old_page->parent_page_num;
    if(new_internal_page_num>0){
    	file_write_page(new_internal_page_num,(page_t*)new_internal);
    	if(error_check==false){
    		file_write_page(old_page_num,(page_t*)old_page);
    		if(error_check==false){
                internal_page_t* right_page=(internal_page_t*)malloc(sizeof(internal_page_t));
                memset(right_page,0,sizeof(internal_page_t));
                file_read_page(right_page_num,(page_t*)right_page);
                if(right_page->isLeaf){
                    if(left_index+1<split-1){//right page in old_page
                        db_set_right_sibling_insert(old_page_num,right_page_num);
                    }
                    else{//right page in new_internal
                        db_set_right_sibling_insert(new_internal_page_num,right_page_num);
                    }
                }
                free(right_page);
    			free(new_internal);
    			return db_insert_into_parent(parent_page_num,old_page_num,new_internal_page_num,k_prime);
    		}
    	}
    }
    return -1;

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
int db_insert_into_parent(pagenum_t parent_page_num,pagenum_t left_page_num,pagenum_t right_page_num,int64_t key){

    // int left_index;
    // node * parent;

    // parent = left->parent;
    int left_index;
    int res;

    /* Case: new root. */

    // if (parent == NULL)
    //     return insert_into_new_root(left, key, right);
    if(parent_page_num==0){
        return db_insert_into_new_root(left_page_num,right_page_num,key);
    }

    	/* Case: leaf or node. (Remainder of
     	* function body.)  
     	*/

    	/* Find the parent's pointer to the left 
     	* node.
     	*/
    internal_page_t* parent_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(parent_page,0,sizeof(internal_page_t));
    file_read_page(parent_page_num,(page_t*)parent_page);
    if(error_check==false){
    	left_index = db_get_left_index(parent_page_num,left_page_num);
    	if(left_index>=-1){
    			/* Simple case: the new key fits into the node. 
     			*/
    		if (parent_page->number_of_keys < MAX_KEY_INTL){
        		res=db_insert_into_internal(parent_page_num,left_page_num,right_page_num,key,left_index,parent_page);
        		free(parent_page);
        		return res;
        	}

    			/* Harder case:  split a node in order 
     			* to preserve the B+ tree properties.
     			*/
			res=db_insert_into_internal_after_splitting(parent_page_num, right_page_num,key,left_index,parent_page);
            free(parent_page);
            return res;
		}
    }
    return -1;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
int db_insert_into_new_root(pagenum_t left_page_num, pagenum_t right_page_num, int64_t key) {

    // node * root = make_node();
    // root->keys[0] = key;
    // root->pointers[0] = left;
    // root->pointers[1] = right;
    // root->num_keys++;
    // root->parent = NULL;
    // left->parent = root;
    // right->parent = root;
    // return root;
    internal_page_t* new_root_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    internal_page_t* left_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    internal_page_t* right_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(new_root_page,0,sizeof(internal_page_t));
    memset(left_page,0,sizeof(internal_page_t));
    memset(right_page,0,sizeof(internal_page_t));
    init_internal(new_root_page);
    file_read_page(left_page_num,(page_t*)left_page);
    if(error_check==false){
    	file_read_page(right_page_num,(page_t*)right_page);
    	if(error_check==false){
    		new_root_page->key_page[0].key=key;
    		new_root_page->key_page[0].page_num=right_page_num;
    		new_root_page->left_most_page_num=left_page_num;
    		new_root_page->number_of_keys++;
            pagenum_t new_root_page_num=file_alloc_page();
            if(left_page->isLeaf) ((leaf_page_t*)left_page)->right_sibling_page_num=right_page_num;
            left_page->parent_page_num=new_root_page_num;
            right_page->parent_page_num=new_root_page_num;
            file_write_page(new_root_page_num,(page_t*)new_root_page);
            file_set_root_page_num(new_root_page_num);
    		if(error_check==false){
    			file_write_page(right_page_num,(page_t*)right_page);
    			if(error_check==false){
                    file_write_page(left_page_num,(page_t*)left_page);
                    free(left_page);
                    left_page=NULL;
                    free(right_page);
                    right_page=NULL;
                    free(new_root_page);
                    new_root_page=NULL;
    				return 0;
    			}
    		}
    	}
	}
	if(left_page!=NULL)free(left_page);
	if(right_page!=NULL)free(right_page);
    if(new_root_page!=NULL) free(new_root_page);
	return -1;
}



/* First insertion:
 * start a new tree.
 */
int db_start_new_tree(int64_t key, char* value) {

    // node * root = make_leaf();
    // root->keys[0] = key;
    // root->pointers[0] = pointer;
    // root->pointers[order - 1] = NULL;
    // root->parent = NULL;
    // root->num_keys++;
    leaf_page_t* new_root_page=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    memset(new_root_page,0,sizeof(leaf_page_t));
    init_leaf(new_root_page);
    new_root_page->key_value[0].key=key;
    strncpy(new_root_page->key_value[0].value,value,strlen(value)+1);
    new_root_page->number_of_keys++;
    file_write_root_page(file_get_root_page_num(),(page_t*)new_root_page);
    free(new_root_page);
    if(error_check==false) return 0;
    else return -1;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int db_insert(int64_t key, char* value) {
    leaf_page_t* leaf_page=NULL;

    int res=-1;
    if(!isOpen) return -1;

    /* The current implementation ignores
     * duplicates.
     */

    if (db_find(key, value)==0) return -1;


    /* Case: the tree does not exist yet.
     * Start a new tree.
     */
    root_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(root_page,0,sizeof(internal_page_t));
    file_read_root_page(((page_t**)&root_page));

    if(error_check==false)
    {
    	if (root_page == NULL){
        	res=db_start_new_tree(key, value);
    	}
   		else{
    		/* Case: the tree already exists.
     		* (Rest of function body.)
     		*/
    		int64_t leaf_page_num = db_find_leaf(key);
    		if(leaf_page_num>0){
    			/* Case: leaf has room for key and pointer.
     			*/
    			leaf_page=(leaf_page_t*)malloc(sizeof(leaf_page_t));
                memset(leaf_page,0,sizeof(leaf_page_t));
    			file_read_page(leaf_page_num,(page_t*)leaf_page);
    			if(error_check==false){
    				if(leaf_page->number_of_keys<MAX_KEY_LEAF) {
                        res=db_insert_into_leaf(leaf_page_num,leaf_page,key,value);
                        free(leaf_page);
                        leaf_page=NULL;
                    }
    				else {
                        res=db_insert_into_leaf_after_splitting(leaf_page_num,leaf_page,key,value);
                        free(leaf_page);
                        leaf_page=NULL;
                    }
    			}
    			else{
    				printf("db_insert, read leaf");
    			}
    		}else{
                printf("fail to find leaf");
            }

     	}
    }else{
    	printf("db_insert, read root");
    }
    free(root_page);
    return res;
}

int db_set_right_sibling_insert(int64_t parent_page_num, int64_t child_page_num){
    internal_page_t* parent_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(parent_page,0,sizeof(internal_page_t));
    file_read_page(parent_page_num,(page_t*)parent_page);
    int i;
    int res=0;
    if(parent_page->left_most_page_num==child_page_num){
        i=-1;
    }
    else{
        for(i=0;i<parent_page->number_of_keys;i++){
            if(parent_page->key_page[i].page_num==child_page_num) break;
        }
    }

    leaf_page_t* before_child=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    leaf_page_t* child=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    memset(before_child,0,sizeof(leaf_page_t));
    memset(child,0,sizeof(leaf_page_t));

    if(parent_page->left_most_page_num!=child_page_num && i<parent_page->number_of_keys && i>0){
        file_read_page(parent_page->key_page[i-1].page_num,(page_t*)before_child);
        file_read_page(child_page_num,(page_t*)child);
        if(error_check==false){
            child->right_sibling_page_num=before_child->right_sibling_page_num;
            before_child->right_sibling_page_num=child_page_num;
            file_write_page(parent_page->key_page[i-1].page_num,(page_t*)before_child);
            if(error_check==false) res=1;
            else res=-1;
        }else res=-1;
    }
    else if(i==0){
        file_read_page(parent_page->left_most_page_num,(page_t*)before_child);
        file_read_page(child_page_num,(page_t*)child);
        if(error_check==false){
            child->right_sibling_page_num=before_child->right_sibling_page_num;
            before_child->right_sibling_page_num=child_page_num;
            file_write_page(parent_page->left_most_page_num,(page_t*)before_child);
            if(error_check==false) res=1;
            else res=-1;
        }else res=-1;
    }
    if(i==parent_page->number_of_keys) res=-1;
    free(parent_page);
    free(before_child);
    free(child);
    return res;
}

void print_internal(internal_page_t* internal_page){
    if(internal_page!=NULL){
        //printf("\nprint internal\n");
        printf("parent_page_num %ld\n",internal_page->parent_page_num);
        printf("isleaf %d\n",internal_page->isLeaf);
        printf("number_of_keys %d\n",internal_page->number_of_keys);
        printf("left_most_page_num %ld\n",internal_page->left_most_page_num);
        for(int i=0;i<internal_page->number_of_keys;i++){
            printf("%ld %ld\n",internal_page->key_page[i].key,internal_page->key_page[i].page_num);
        }
    }
}

void print_leaf(leaf_page_t* leaf_page){
    if(leaf_page!=NULL){
        printf("parent_page_num %ld\n",leaf_page->parent_page_num);
        printf("isleaf %d\n",leaf_page->isLeaf);
        printf("number_of_keys %d\n",leaf_page->number_of_keys);
        printf("right sibling %d\n",leaf_page->right_sibling_page_num);
        for(int i=0;i<leaf_page->number_of_keys;i++){
            printf("%ld %s\n",leaf_page->key_value[i].key, leaf_page->key_value[i].value);

        }
    }
}

void print_page(){
    int64_t num_of_pages=file_get_number_of_pages();
    internal_page_t* cur_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(cur_page,0,sizeof(internal_page_t));
    file_read_page(0, (page_t*)cur_page);
    printf("root_page:%ld\n",((page_t*)cur_page)->root_page);
    int64_t root_page_num=((page_t*)cur_page)->root_page;
    file_read_page(((page_t*)cur_page)->root_page, (page_t*)cur_page);
    print_internal(cur_page);
    for(int64_t i=0;i<=(num_of_pages>root_page_num ? num_of_pages : root_page_num);i++){
        file_read_page(i,(page_t*)cur_page);
        printf("pagenum:%ld\n",i);
        if(i==0){
            printf("next free page:%ld\n",((page_t*)cur_page)->next_free_page);
            printf("root page :%ld\n",((page_t*)cur_page)->root_page);
            printf("number of page:%ld\n",((page_t*)cur_page)->number_of_page);
            continue;
        }
        if(!cur_page->isLeaf) print_internal(cur_page);
        else print_leaf((leaf_page_t*)cur_page);
    }
    free(cur_page);
}



// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */


leaf_page_t* db_remove_entry_from_leaf(int64_t leaf_page_num, int64_t key) {

    int i, num_pointers;

    leaf_page_t* leaf_page=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    memset(leaf_page,0,sizeof(leaf_page_t));
    file_read_page(leaf_page_num,(page_t*)leaf_page);
    if(error_check==false){
        i = 0;
        while (leaf_page->key_value[i].key != key && i<leaf_page->number_of_keys){
            i++;
        }
        if(i==leaf_page->number_of_keys){
            free(leaf_page);
            return NULL;
        }
        for (++i; i <leaf_page->number_of_keys; i++)
           leaf_page->key_value[i - 1] = leaf_page->key_value[i];
       leaf_page->number_of_keys--;
       return leaf_page;
    }
    free(leaf_page);
    return NULL;

    // Remove the key and shift other keys accordingly.
    // i = 0;
    // while (n->keys[i] != key)
    //     i++;
    // for (++i; i < n->num_keys; i++)
    //     n->keys[i - 1] = n->keys[i];

    // // Remove the pointer and shift other pointers accordingly.
    // // First determine number of pointers.
    // num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
    // i = 0;
    // while (n->pointers[i] != pointer)
    //     i++;
    // for (++i; i < num_pointers; i++)
    //     n->pointers[i - 1] = n->pointers[i];


    // // One key fewer.
    // n->num_keys--;

    // // Set the other pointers to NULL for tidiness.
    // // A leaf uses the last pointer to point to the next leaf.
    // if (n->is_leaf)
    //     for (i = n->num_keys; i < order - 1; i++)
    //         n->pointers[i] = NULL;
    // else
    //     for (i = n->num_keys + 1; i < order; i++)
    //         n->pointers[i] = NULL;
}


int db_adjust_root(leaf_page_t* leaf_page,int64_t leaf_page_num) {

    int i;

    /*Case: empty root.
     */
    if(leaf_page->number_of_keys==0){
        //printf("init_file");
        //file_initialize();
        if(error_check==true) return -1;
        return 0;
    }
    /* Case: Nonempty root. 
     */
    //file_write_page(leaf_page_num,(page_t*)leaf_page_num);
    if(error_check==true) return -1;
    return 0;
}



int db_delete_entry(int64_t leaf_page_num,int64_t key) {

    int res;
   //printf("in delete entry");
    leaf_page_t* leaf_page=db_remove_entry_from_leaf(leaf_page_num,key);

    if(leaf_page==NULL) return -1;

     /* Case:  deletion from the root. 
     */
    if (leaf_page_num==file_get_root_page_num()){
        res=db_adjust_root(leaf_page, leaf_page_num);
        file_write_page(leaf_page_num,(page_t*)leaf_page);
    }
    else{
        if(leaf_page->number_of_keys==0){
            res=db_set_right_sibling_delete(leaf_page->parent_page_num, leaf_page_num);
            if(res!=-1){
                res=db_delayed_merge_internal(leaf_page->parent_page_num, leaf_page_num);
            }
        }
        else {
            res=0;
            file_write_page(leaf_page_num,(page_t*)leaf_page);
        }
    }
    free(leaf_page);
    return res;
}


int db_delayed_merge_internal(int64_t parent_page_num, int64_t child_page_num){
    int i,j;
    int deletion_index,neighbor_index;
    int res=-1;
    //printf("in delayed merge");
    internal_page_t* parent_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(parent_page,0,sizeof(internal_page_t));
    file_read_page(parent_page_num,(page_t*)parent_page);
    if(error_check==false){
        if(parent_page->number_of_keys==1){
            //printf("root_page_num:%ld\n",file_get_root_page_num());
            //print_page();
            printf("\n");
            if(parent_page->parent_page_num==0){
                // internal_page_t* temp=(internal_page_t*)malloc(sizeof(internal_page_t));
                // memset(temp, 0 ,sizeof(internal_page_t));
                // printf("print_page\n");
                // file_read_page(parent_page->left_most_page_num, (page_t*)temp);
                // print_internal(temp);
                // for(int k=0;k<parent_page->number_of_keys;k++){
                //     printf("print_page\n");
                //     file_read_page(parent_page->key_page[i].page_num, (page_t*)temp);
                //     print_internal(temp);
                // }
                // free(temp);
                internal_page_t* new_root_page=(internal_page_t*)malloc(sizeof(internal_page_t));
                memset(new_root_page,0,sizeof(internal_page_t));
                if(parent_page->left_most_page_num==child_page_num){
                    file_read_page(parent_page->key_page[0].page_num,(page_t*)new_root_page);
                    new_root_page->parent_page_num=0;
                    file_write_page(parent_page->key_page[0].page_num,(page_t*)new_root_page);
                    file_set_root_page_num(parent_page->key_page[0].page_num);
                }
                else if(parent_page->key_page[0].page_num==child_page_num){
                    file_read_page(parent_page->left_most_page_num,(page_t*)new_root_page);
                    new_root_page->parent_page_num=0;
                    //print_internal(new_root_page);
                    file_write_page(parent_page->left_most_page_num,(page_t*)new_root_page);
                    file_set_root_page_num(parent_page->left_most_page_num);
                }
                else{
                    printf("invalid child page");
                    free(parent_page);
                    free(new_root_page);
                    return -1;
                }
                print_internal(new_root_page);
                file_free_page(child_page_num);
                file_free_page(parent_page_num);
                free(parent_page);
                free(new_root_page);
                return 0;
            }

            pagenum_t p_parent_page_num=parent_page->parent_page_num;
            internal_page_t* p_parent_page=(internal_page_t*)malloc(sizeof(internal_page_t));
            memset(p_parent_page,0,sizeof(internal_page_t));
            file_read_page(p_parent_page_num,(page_t*)p_parent_page);

            if(p_parent_page->left_most_page_num==parent_page_num) deletion_index=-1;
            else{
                for(i=0;i<p_parent_page->number_of_keys;i++){
                    if(p_parent_page->key_page[i].page_num==parent_page_num) break;
                }
                if(i==p_parent_page->number_of_keys) {
                    printf("no key, p_parnet_page->number_of_keys:%ld\n",p_parent_page->number_of_keys);
                    return -1;
                }
                deletion_index=i;
            }

            neighbor_index=deletion_index+1;
            if(deletion_index==p_parent_page->number_of_keys-1) neighbor_index=deletion_index-1;

            int64_t new_key=p_parent_page->key_page[neighbor_index<deletion_index ? deletion_index:neighbor_index].key;
            internal_page_t* neighbor_index_page=(internal_page_t*)malloc(sizeof(internal_page_t));
            memset(neighbor_index_page,0,sizeof(internal_page_t));
            file_read_page(p_parent_page->key_page[neighbor_index].page_num,(page_t*)neighbor_index_page);

            if(neighbor_index_page->number_of_keys==MAX_KEY_INTL){//neighbor is full: split and Insert

                page_pair temp[MAX_KEY_INTL+1];
                if(deletion_index<neighbor_index){
                    temp[0].key=new_key;
                    temp[0].page_num=neighbor_index_page->left_most_page_num;
                    for(i=1;i<=neighbor_index_page->number_of_keys;i++)
                        temp[i]=neighbor_index_page->key_page[i-1];
                }
                else{
                    for(i=0;i<neighbor_index_page->number_of_keys;i++){
                        temp[i]=neighbor_index_page->key_page[i];
                    }
                    temp[neighbor_index_page->number_of_keys].key=new_key;
                    if(child_page_num==parent_page->left_most_page_num) temp[neighbor_index_page->number_of_keys].page_num=parent_page->key_page[0].page_num;
                    else temp[neighbor_index_page->number_of_keys].page_num=parent_page->left_most_page_num;
                }

                //deletion_page==parent_page

                parent_page->number_of_keys=0;
                neighbor_index_page->number_of_keys=0;

                int split=cut(MAX_KEY_INTL);
                //To Do
                internal_page_t* child_page=(internal_page_t*)malloc(sizeof(internal_page_t));
                memset(child_page, 0, sizeof(internal_page_t));

                if(deletion_index<neighbor_index){
                    if(child_page_num==parent_page->left_most_page_num) {
                        parent_page->left_most_page_num=parent_page->key_page[0].page_num;
                    }
                    for(i=0;i<split;i++){
                        parent_page->key_page[i]=temp[i];
                        parent_page->number_of_keys++;
                        file_read_page(parent_page->key_page[i].page_num, (page_t*)child_page);
                        child_page->parent_page_num=parent_page_num;
                        file_write_page(parent_page->key_page[i].page_num, (page_t*)child_page);
                    }
                    p_parent_page->key_page[neighbor_index].key=temp[split].key;
                    neighbor_index_page->left_most_page_num=temp[split].page_num;

                    for(++i,j=0;i<MAX_KEY_INTL+1;i++,j++){
                        neighbor_index_page->key_page[j]=temp[i];
                        neighbor_index_page->number_of_keys++;
                    }
                }else{
                    for(i=0;i<split;i++){
                        neighbor_index_page->key_page[i]=temp[i];
                        neighbor_index_page->number_of_keys++;
                    }
                    p_parent_page->key_page[deletion_index].key=temp[split].key;
                    parent_page->left_most_page_num=temp[split].page_num;
                    file_read_page(parent_page->left_most_page_num, (page_t*)child_page);
                    child_page->parent_page_num=parent_page_num;
                    file_write_page(parent_page->left_most_page_num,(page_t*)child_page);

                    for(++i,j=0;i<MAX_KEY_INTL+1;i++,j++){
                        parent_page->key_page[j]=temp[i];
                        parent_page->number_of_keys++;
                        file_read_page(parent_page->key_page[i].page_num, (page_t*)child_page);
                        child_page->parent_page_num=parent_page_num;
                        file_write_page(parent_page->key_page[i].page_num, (page_t*)child_page);
                    }
                }
                free(child_page);

                file_free_page(child_page_num);

                file_write_page(p_parent_page->key_page[deletion_index].page_num,(page_t*)parent_page);
                if(error_check==false){
                    file_write_page(p_parent_page->key_page[neighbor_index].page_num,(page_t*)neighbor_index_page);
                    if(error_check==false){
                        file_write_page(p_parent_page_num,(page_t*)p_parent_page);
                        if(error_check==false){
                            res=0;
                        }
                    }
                }
                free(parent_page);
                parent_page=NULL;
                free(neighbor_index_page);
                neighbor_index_page=NULL;
                free(p_parent_page);
                p_parent_page=NULL;
                }
                else{//neighbor is not full: just attach 
                    //To Do
                    internal_page_t* child_page=(internal_page_t*)malloc(sizeof(internal_page_t));
                    memset(child_page, 0 ,sizeof(internal_page_t));
                    if(deletion_index<neighbor_index){
                        for(i=neighbor_index_page->number_of_keys;i>0;i--){
                            neighbor_index_page->key_page[i]=neighbor_index_page->key_page[i-1];
                        }
                        neighbor_index_page->key_page[0].page_num=neighbor_index_page->left_most_page_num;
                        neighbor_index_page->key_page[0].key=new_key;
                        if(child_page_num==parent_page->left_most_page_num) {
                            neighbor_index_page->left_most_page_num=parent_page->key_page[0].page_num;
                        }
                        else neighbor_index_page->left_most_page_num=parent_page->left_most_page_num;

                        file_read_page(neighbor_index_page->left_most_page_num,(page_t*)child_page);
                        child_page->parent_page_num=p_parent_page->key_page[neighbor_index].page_num;
                        file_write_page(neighbor_index_page->left_most_page_num,(page_t*)child_page);
                        neighbor_index_page->number_of_keys++;
                    }
                    else{
                        neighbor_index_page->key_page[neighbor_index_page->number_of_keys].key=new_key;
                        if(child_page_num==parent_page->left_most_page_num) neighbor_index_page->key_page[neighbor_index_page->number_of_keys].page_num=parent_page->key_page[0].page_num;
                        else neighbor_index_page->key_page[neighbor_index_page->number_of_keys].page_num=parent_page->left_most_page_num;

                        file_read_page(neighbor_index_page->key_page[neighbor_index_page->number_of_keys].page_num,(page_t*)child_page);
                        child_page->parent_page_num=p_parent_page->key_page[neighbor_index].page_num;
                        file_write_page(neighbor_index_page->key_page[neighbor_index_page->number_of_keys].page_num,(page_t*)child_page);
                        neighbor_index_page->number_of_keys++;
                    }

                    file_write_page(p_parent_page->key_page[neighbor_index].page_num,(page_t*)neighbor_index_page);

                    // if(deletion_index<neighbor_index){
                    //     int i=deletion_index;
                    //     if(deletion_index==-1){
                    //         p_parent_page->left_most_page_num=p_parent_page->key_page[0].page_num;
                    //         i++;
                    //     }
                    //     for(i;i<p_parent_page->number_of_keys-1;i++){
                    //         p_parent_page->key_page[i]=p_parent_page->key_page[i+1];
                    //     }
                    //     p_parent_page->number_of_keys--;
                    // }else{
                    //     p_parent_page->number_of_keys--;
                    // }
                    file_free_page(child_page_num);
                    //file_free_page(parent_page_num);
                    if(error_check==false){
                        file_write_page(p_parent_page_num,(page_t*)p_parent_page);
                        if(error_check==false){
                            return db_delayed_merge_internal(p_parent_page_num,parent_page_num);
                        }
                    }
                    free(parent_page);
                    parent_page=NULL;
                    free(neighbor_index_page);
                    neighbor_index_page=NULL;
                    free(p_parent_page);
                    p_parent_page=NULL;
                }
        }
        else{//number of keys > 1
            if(parent_page->left_most_page_num==child_page_num) deletion_index=-1;
            else{
                for(i=0;i<parent_page->number_of_keys;i++){
                    if(parent_page->key_page[i].page_num==child_page_num) break;
                }
                deletion_index=i;
            }
            if(deletion_index==parent_page->number_of_keys) {
                printf("parent_page->number_of_keys:%ld\n",parent_page->number_of_keys);
                printf("no key number_of_keys>1\n");
                print_internal(parent_page);
                return -1;
            }
            else{
                if(deletion_index==-1){
                    parent_page->left_most_page_num=parent_page->key_page[0].page_num;
                    deletion_index++;
                }
                for(i=deletion_index;i<parent_page->number_of_keys-1;i++){
                        parent_page->key_page[i]=parent_page->key_page[i+1];
                }
            }
            parent_page->number_of_keys--;
            file_free_page(child_page_num);
            file_write_page(parent_page_num,(page_t*)parent_page);
            free(parent_page);
            res=0;
        }
    }
    return res;
}

int db_set_right_sibling_delete(int64_t parent_page_num, int64_t child_page_num){
    internal_page_t* parent_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(parent_page,0 ,sizeof(internal_page_t));
    file_read_page(parent_page_num,(page_t*)parent_page);
    int i;
    if(parent_page->left_most_page_num==child_page_num){
        i=-1;
    }
    else {
        for(i=0;i<parent_page->number_of_keys;i++){
            if(parent_page->key_page[i].page_num==child_page_num) break;
        }
    }
    leaf_page_t* before_child=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    leaf_page_t* child=(leaf_page_t*)malloc(sizeof(leaf_page_t));
    memset(before_child,0,sizeof(leaf_page_t));
    memset(child,0,sizeof(leaf_page_t));
    if(0<i && i<parent_page->number_of_keys) file_read_page(parent_page->key_page[i-1].page_num,(page_t*)before_child);
    else if(i==0) file_read_page(parent_page->left_most_page_num,(page_t*)before_child);
    else{
        free(before_child);
        free(child);
        free(parent_page);
        if(i==-1) return 0;
        else return -1;
    }
    file_read_page(parent_page->key_page[i].page_num,(page_t*)child);
    before_child->right_sibling_page_num=child->right_sibling_page_num;
    if(0<i && i<parent_page->number_of_keys) file_write_page(parent_page->key_page[i-1].page_num,(page_t*)before_child);
    else if(i==0) file_write_page(parent_page->left_most_page_num,(page_t*)before_child);
    free(before_child);
    free(child);
    free(parent_page);
    return 0;
}




/* Master deletion function.
 */
int db_delete(int64_t key) {

    if(!isOpen) return -1;
    
    root_page=(internal_page_t*)malloc(sizeof(internal_page_t));
    memset(root_page,0,sizeof(internal_page_t));
    file_read_root_page((page_t**)&root_page);
    if(root_page==NULL) return -1; //no_data

    int64_t leaf_page_num=db_find_leaf(key);
    // if (key_record != NULL && key_leaf != NULL) {
    //     root = delete_entry(root, key_leaf, key, key_record);
    //     free(key_record);
    // }
    int res=-1;
    //printf("leaf_page_num:%ld\n",leaf_page_num);
    if(leaf_page_num>0){
        res=db_delete_entry(leaf_page_num,key);
    }
    free(root_page);
    root_page=NULL;
    return res;
}

