#include "bpt.h"
#include <time.h>
#include <stdlib.h>
#define THREAD_COUNT 20
// MAIN

typedef struct arg_t{
	int table_id;
	int64_t key;
	char ret_val[100];
	int trx_id;
}arg_t;

int commit_done=0;

void* test_func(void* vp){
	int trx_id=trx_begin();
	printf("%d is begin with %ld\n", trx_id, ((arg_t*)vp)->key);
	((arg_t*)vp)->trx_id=trx_id;
	int r;
    int done=0;
	r=db_find(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
    done=1;
	if(r==0){
        printf("trx: %d successs find %d\n",trx_id, ((arg_t*)vp)->key);
    }
    else{
        if(r==-1) printf("trx: %d fail to find %d\n", trx_id, ((arg_t*)vp)->key);
        if(r==-2) printf("trx: %d abort to find %d\n", trx_id, ((arg_t*)vp)->key);
    }
    if(r!=-2 && done==1){
        //printf("trx_id: %d going update\n", trx_id);
        done=0;
    	r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
        done=1;
    	if(r==0){
            printf("trx: %d successs update %d\n",trx_id, ((arg_t*)vp)->key);
    	}
    	else{
        	if(r==-1) printf("trx: %d fail to update %d\n", trx_id, ((arg_t*)vp)->key);
        	if(r==-2) printf("trx: %d abort to update %d\n", trx_id, ((arg_t*)vp)->key);
    	}
    	if(r!=-2){
            //printf("trx_id: %d going update\n", trx_id);
    		r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
    		if(r==0){
       			 printf("trx: %d successs update %d\n",trx_id, ((arg_t*)vp)->key);
    		}
    		else{
        		if(r==-1) printf("trx: %d fail to update %d\n",trx_id, ((arg_t*)vp)->key);
        		if(r==-2) printf("trx: %d abort to update %d\n",trx_id, ((arg_t*)vp)->key);
    		}
    		if(r!=-2){
    			trx_commit(trx_id);
    			printf("trx:%d commit done\n", trx_id);
    			commit_done=trx_id;
    			print_trx(1,THREAD_COUNT);
    		}
    	}
    }
    trx_id=trx_begin();
    printf("%d is begin with %ld\n", trx_id, ((arg_t*)vp)->key);
    ((arg_t*)vp)->trx_id=trx_id;
    r=db_find(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
    done=1;
    if(r==0){
        printf("trx: %d successs find %d\n",trx_id, ((arg_t*)vp)->key);
    }
    else{
        if(r==-1) printf("trx: %d fail to find %d\n", trx_id, ((arg_t*)vp)->key);
        if(r==-2) printf("trx: %d abort to find %d\n", trx_id, ((arg_t*)vp)->key);
    }
    if(r!=-2 && done==1){
        //printf("trx_id: %d going update\n", trx_id);
        done=0;
        r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
        done=1;
        if(r==0){
            printf("trx: %d successs update %d\n",trx_id, ((arg_t*)vp)->key);
        }
        else{
            if(r==-1) printf("trx: %d fail to update %d\n", trx_id, ((arg_t*)vp)->key);
            if(r==-2) printf("trx: %d abort to update %d\n", trx_id, ((arg_t*)vp)->key);
        }
        if(r==-2) printf("trx: %d abort to update %d\n", trx_id, ((arg_t*)vp)->key);
        if(r!=-2){
            //printf("trx_id: %d going update\n", trx_id);
            r=db_find(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
            if(r==0){
                 printf("trx: %d successs find %d\n",trx_id, ((arg_t*)vp)->key);
            }
            else{
                if(r==-1) printf("trx: %d fail to find %d\n",trx_id, ((arg_t*)vp)->key);
                if(r==-2) printf("trx: %d abort to find %d\n",trx_id, ((arg_t*)vp)->key);
            }
            if(r!=-2){
                trx_commit(trx_id);
                printf("trx:%d commit done\n", trx_id);
                commit_done=trx_id;
                print_trx(1,THREAD_COUNT);
            }
        }
    }
}

void* test_func1(void *vp){
    int trx_id=trx_begin();
    printf("%d is begin with %ld\n", trx_id, ((arg_t*)vp)->key);
    ((arg_t*)vp)->trx_id=trx_id;
    ((arg_t*)vp)->key=rand()%7000;
    int r;
    int done=0;
    sprintf(((arg_t*)vp)->ret_val,"%d  update is value", ((arg_t*)vp)->key);
    r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
    done=1;
    if(r==0){
        printf("trx: %d successs up %d:%s\n ",trx_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val);

    }
    else{
        if(r==-1) printf("trx: %d fail to up %d\n", trx_id, ((arg_t*)vp)->key);
        if(r==-2) printf("trx: %d abort to up %d\n", trx_id, ((arg_t*)vp)->key);
    }
    if(r!=-2 && done==1){
        //printf("trx_id: %d going update\n", trx_id);
        done=0;
        ((arg_t*)vp)->key=rand()%7000;
        sprintf(((arg_t*)vp)->ret_val,"%d  update is value", ((arg_t*)vp)->key);
        r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
        done=1;
        if(r==0){
            printf("trx: %d successs up %d:%s\n ",trx_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val);
        }
        else{
            if(r==-1) printf("trx: %d fail to up %d\n", trx_id, ((arg_t*)vp)->key);
            if(r==-2) printf("trx: %d abort to up %d\n", trx_id, ((arg_t*)vp)->key);
        }
        if(r!=-2){
            //printf("trx_id: %d going update\n", trx_id);
            ((arg_t*)vp)->key=rand()%7000;
            sprintf(((arg_t*)vp)->ret_val,"%d  update is value", ((arg_t*)vp)->key);
            r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
            if(r==0){
                printf("trx: %d successs up %d:%s\n ",trx_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val);
            }
            else{
                if(r==-1) printf("trx: %d fail to up %d\n",trx_id, ((arg_t*)vp)->key);
                if(r==-2) printf("trx: %d abort to up %d\n",trx_id, ((arg_t*)vp)->key);
            }
            if(r!=-2){
                trx_commit(trx_id);
                printf("trx:%d commit done\n", trx_id);
                commit_done=trx_id;
                print_trx(1,THREAD_COUNT);
            }
        }
    }
    trx_id=trx_begin();
    printf("%d is begin with %ld\n", trx_id, ((arg_t*)vp)->key);
    ((arg_t*)vp)->trx_id=trx_id;
    ((arg_t*)vp)->key=rand()%7000;
    sprintf(((arg_t*)vp)->ret_val,"%d  update is value", ((arg_t*)vp)->key);
    r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
    done=1;
    if(r==0){
        printf("trx: %d successs up %d:%s\n ",trx_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val);
    }
    else{
        if(r==-1) printf("trx: %d fail to find %d\n", trx_id, ((arg_t*)vp)->key);
        if(r==-2) printf("trx: %d abort to find %d\n", trx_id, ((arg_t*)vp)->key);
    }
    if(r!=-2 && done==1){
        //printf("trx_id: %d going update\n", trx_id);
        done=0;
        ((arg_t*)vp)->key=rand()%7000;
        sprintf(((arg_t*)vp)->ret_val,"%d  update is value", ((arg_t*)vp)->key);
        r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
        done=1;
        if(r==0){
            printf("trx: %d successs up %d:%s\n",trx_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val);
        }
        else{
            if(r==-1) printf("trx: %d fail to find %d\n", trx_id, ((arg_t*)vp)->key);
            if(r==-2) printf("trx: %d abort to find %d\n", trx_id, ((arg_t*)vp)->key);
        }
        if(r==-2) printf("trx: %d abort to update %d\n", trx_id, ((arg_t*)vp)->key);
        if(r!=-2){
            ((arg_t*)vp)->key=rand()%7000;
            //printf("trx_id: %d going update\n", trx_id);
            sprintf(((arg_t*)vp)->ret_val,"%d  update is value", ((arg_t*)vp)->key);
            r=db_update(((arg_t*)vp)->table_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val, ((arg_t*)vp)->trx_id);
            if(r==0){
                printf("trx: %d successs up %d:%s\n",trx_id, ((arg_t*)vp)->key, ((arg_t*)vp)->ret_val);
            }
            else{
                if(r==-1) printf("trx: %d fail to find %d\n",trx_id, ((arg_t*)vp)->key);
                if(r==-2) printf("trx: %d abort to find %d\n",trx_id, ((arg_t*)vp)->key);
            }
            if(r!=-2){
                trx_commit(trx_id);
                printf("trx:%d commit done\n", trx_id);
                commit_done=trx_id;
                print_trx(1,THREAD_COUNT);
            }
        }
    }
}

int main( int argc, char ** argv ){
    int res;
    char instruction;
    char filepath[21];

    int64_t key=1;
    char value[120]="";
    int hash[10001]={0,};
    int count=0;
    init_db(8);
    int id=open_table("simple.db");
    int r=0;
    srand(time(NULL));
    pthread_t thread[THREAD_COUNT];
    // if(r==0){
    //  for(int i=0;i<10000;i++){
    //      r=db_delete(i);
    //      if(r==0) printf("1 success delete %d\n",i);
    //      else{
    //          printf("1 fail to delete %d\n",i);
    //          break;
    //      }
    //  }
    // }
    printf("%d\n",id);
    while(1){
        if(hash[count]!=1) {
            memset(value,0,sizeof(char)*120);
            sprintf(value, "%d is value", count);
            r=db_insert(id,count,value);
            if(r==0) {
                printf("1 successs insert %d\n",count);
                //print_buf();
            }
            else {
                printf("1 fail insert %d\n", count);
                break;
            }
            hash[count]=1;
            count++;
        }
        if(count==7000) break;
    }
    //print_page(id);
    arg_t arg[THREAD_COUNT];
    for(int i=0;i<THREAD_COUNT;i++){
        memset(&arg[i],0, sizeof(arg_t));
    	arg[i].table_id=id;
        arg[i].key=1;
        sprintf(arg[i].ret_val, "%d is update value", arg[i].key);
    	pthread_create(&thread[i], 0, test_func1, (void*)&arg[i]);
    }

    for(int i=0;i<THREAD_COUNT;i++){
    	pthread_join(thread[i],NULL);
    }
    r=db_find(id, 1, value, 0);
    if(r==0){
    	printf("\n%s %d\n", value, commit_done);
    }


    //print_page(id);
    // if(r==0){
    //     for(int i=0;i<10001;i++){
    //         if(hash[i]==1){
    //             r=db_find(id,i,value);
    //             if(r==0) printf("1 success find %d\n",i);
    //             else{
    //                 printf("1 fail to find %d\n",i);
    //                 break;
    //             }
    //         }
    //     }
    // }
    // count=0;
    // if(r==0){
    //     for(int i=0;i<10001;i++){
    //         if(hash[i]==1){
    //             hash[i]=0;
    //             r=db_delete(id, i);
    //             //print_buf();
    //             if(r==0) printf("1 success delete %d\n",i);
    //             else{
    //                 printf("1 fail to delete %d\n",i);
    //                 break;
    //             }
    //             count++;
    //         }
    //         if(count==4000) break;
    //     }
    // }
    // count=0;
    // if(r!=0) return 0;
    // while(1){
    //     int res=rand()%10001;
    //     if(hash[res]!=1) {
    //         memset(value,0,sizeof(char)*120);
    //         sprintf(value, "%d is value", res);
    //         r=db_insert(id,res,value);
    //         if(r==0) printf("2 successs insert %d\n",res);
    //         else {
    //             printf("2 fail insert %d\n", res);
    //             break;
    //         }
    //         hash[res]=1;
    //         count++;
    //     }
    //     if(count==3000) break;
    // }
    //print_page();
    // if(r==0){
    //     for(int i=0;i<10001;i++){
    //         if(hash[i]==1){
    //             r=db_find(id, i,value);
    //             if(r==0) printf("2 success find %d\n",i);
    //             else{
    //                 printf("2 fail to find %d\n",i);
    //                 break;
    //             }
    //         }
    //     }
    // }
    // if(r==0){
    //     for(int i=0;i<10001;i++){
    //         if(hash[i]==1){
    //             r=db_delete(id, i);
    //             //print_buf();
    //             hash[i]=0;
    //             if(r==0) printf("2 success delete %d\n",i);
    //             else{
    //                 printf("2 fail to delete %d\n",i);
    //                 break;
    //             }
    //         }
    //     }
    // }
    close_table(id);
    shutdown_db();
}
