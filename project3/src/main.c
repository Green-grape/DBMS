#include "bpt.h"
#include <time.h>
// MAIN


int main( int argc, char ** argv ) {
    int res;
    char instruction;
    char filepath[21];

    int64_t key=1;
    char value[120]="";
    int hash[10001]={0,};
    int count=0;

    //usage();
    
    // printf("> ");
    // while (1) {
    //     scanf(" %c", &instruction);
    //     switch (instruction) {
    //     case 'o':
    //         scanf(" %[^\n]s",filepath);
    //         printf("%s\n",filepath);
    //         int64_t id=open_table(filepath);
    //         if(id!=-1) printf("table_id: %ld\n",id);
    //         else printf("(%ld)fail to open...\n",id);
    //         break;
    //     case 'i':
    //         scanf(" %ld %[^\n]s", &key,value);
    //         fflush(stdin);
    //         res=db_insert(key,value);
    //         if(res==0) printf("(%d)success!\n",res);
    //         else printf("(%d)fail to insert....\n",res);
    //         break;
    //     case 'f':
    //         scanf(" %ld %[^\n]s", &key,value);
    //         fflush(stdin);
    //         res=db_find(key,value);
    //         if(res==0) printf("(%d)success!\n",res);
    //         else printf("(%d)fail to find....\n",res);
    //         break;
    //     case 'd':
    //         scanf("%ld\n",&key);
    //         res=db_delete(key);
    //         if(res==0) printf("(%d)success!\n",res);
    //         else printf("(%d)fail to delete....\n",res);
    //     default:
    //         break;
    //     }
    //     printf("> ");
    // }
    // printf("\n");
    init_db(4);
    int id=open_table("simple.db");
    int r=0;
    srand(time(NULL));
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
    if(r==0){
        for(int i=0;i<10001;i++){
            if(hash[i]==1){
                r=db_find(id,i,value);
                if(r==0) printf("1 success find %d\n",i);
                else{
                    printf("1 fail to find %d\n",i);
                    break;
                }
            }
        }
    }
    count=0;
    if(r==0){
        for(int i=0;i<10001;i++){
            if(hash[i]==1){
                hash[i]=0;
                r=db_delete(id, i);
                //print_buf();
                if(r==0) printf("1 success delete %d\n",i);
                else{
                    printf("1 fail to delete %d\n",i);
                    break;
                }
                count++;
            }
            if(count==4000) break;
        }
    }
    count=0;
    if(r!=0) return 0;
    while(1){
        int res=rand()%10001;
        if(hash[res]!=1) {
            memset(value,0,sizeof(char)*120);
            sprintf(value, "%d is value", res);
            r=db_insert(id,res,value);
            if(r==0) printf("2 successs insert %d\n",res);
            else {
                printf("2 fail insert %d\n", res);
                break;
            }
            hash[res]=1;
            count++;
        }
        if(count==3000) break;
    }
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
