#include "bpt.h"
#include <time.h>
// MAIN


int main( int argc, char ** argv ) {
    int res;
    char instruction;
    char filepath[100];

    int64_t key=1;
    char value[120]="";
    int hash[10001]={0,};
    int count=1;

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
    open_table("simple.db");
    int r=0;
    srand(time(NULL));
    // if(r==0){
   	// 	for(int i=0;i<10000;i++){
   	// 		r=db_delete(i);
   	// 		if(r==0) printf("1 success delete %d\n",i);
   	// 		else{
   	// 			printf("1 fail to delete %d\n",i);
   	// 			break;
   	// 		}
   	// 	}
   	// }
    int i;
   	// while(1){
   	// 	int res=rand()%10001;
   	// 	if(hash[res]!=1) {
   	// 		memset(value,0,sizeof(char)*120);
   	// 		sprintf(value, "%d is value", res);
   	// 		r=db_insert(res,value);
   	// 		if(r==0) printf("1 successs insert %d\n",res);
   	// 		else {
   	// 			printf("1 fail insert %d\n", res);
   	// 			break;
   	// 		}
   	// 		hash[res]=1;
   	// 		count++;
   	// 	}
   	// 	if(count==6000) break;
   	// }
   	if(r==0){
   		for(i=0;i<7000;i++){
   				r=db_find(i,value);
   				if(r==0) printf("1 success find %d\n",i);
   				else{
   					printf("1 fail to find %d\n",i);
   					break;
   				}
          hash[i]=1;
   			}
   	}
    return 0;
   	count=0;
   	if(r==0){
   		for(int i=0;i<10001;i++){
   			if(hash[i]==1){
   				hash[i]=0;
   				r=db_delete(i);
   				if(r==0) {
            printf("1 success delete %d\n",i);
            //print_page();
          }
   				else{
   					printf("1 fail to delete %d\n",i);
   					break;
   				}
   				count++;
   			}
   			if(count==7000) break;
   		}
   	}
   	count=0;
   	printf("res:%d\n",r);
    print_page();
   	while(1){
   		int res=rand()%10001;
   		if(hash[res]!=1) {
   			memset(value,0,sizeof(char)*120);
   			sprintf(value, "%d is value", res);
   			r=db_insert(res,value);
   			if(r==0) printf("2 successs insert %d\n",res);
   			else {
   				printf("2 fail insert %d\n", res);
   				break;
   			}
   			hash[res]=1;
   			count++;
   		}
   		if(count==3500) break;
   	}
   	//print_page();
   	if(r==0){
   		for(int i=0;i<10001;i++){
   			if(hash[i]==1){
   				r=db_find(i,value);
   				if(r==0) printf("2 success find %d\n",i);
   				else{
   					printf("2 fail to find %d\n",i);
   					break;
   				}
   			}
   		}
   	}
   	if(r==0){
   		for(int i=0;i<3001;i++){
   			if(hash[i]==1){
   				r=db_delete(i);
   				hash[i]=0;
   				if(r==0) printf("2 success delete %d\n",i);
   				else{
   					printf("2 fail to delete %d\n",i);
   					break;
   				}
   			}
   		}
   	}
   	

   	// while(1){
   	// 	int res=rand()%3001;
   	// 	if(hash[res]!=1) {
   	// 		memset(value,0,sizeof(char)*120);
   	// 		sprintf(value, "%d is value", res);
   	// 		r=db_insert(res,value);
   	// 		if(r==0) printf("2 successs insert %d\n",res);
   	// 		else {
   	// 			printf("2 fail insert %d\n", res);
   	// 			break;
   	// 		}
   	// 		hash[res]=1;
   	// 		count++;
   	// 	}
   	// 	if(count==2000) break;
   	// }
   	// if(r==0){
   	// 	for(int i=0;i<3001;i++){
   	// 		if(hash[i]==1){
   	// 			r=db_find(i,value);
   	// 			if(r==0) printf("2 success find %d\n",i);
   	// 			else{
   	// 				printf("2 fail to find %d\n",i);
   	// 				break;
   	// 			}
   	// 		}
   	// 	}
   	// }
   	// if(r==0){
   	// 	for(int i=0;i<3001;i++){
   	// 		if(hash[i]==1){
   	// 			r=db_delete(i);
   	// 			if(r==0) printf("2 success delete %d\n",i);
   	// 			else{
   	// 				printf("2 fail to delete %d\n",i);
   	// 				break;
   	// 			}
   	// 		}
   	// 	}
   	// }

    return EXIT_SUCCESS;
}
