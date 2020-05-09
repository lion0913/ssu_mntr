#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define MAX_TOKEN 20 //입력가능한 최대 토큰 수를 20개로 가정
#define MODIFY 0
#define DELETE 1
#define CREATE 2
#define N -1 

typedef struct f_tree{
        char fname[BUFFER_SIZE];//파일이름
        struct dirent **namelist;
        struct stat statbuf;
        struct f_tree *sibling;
        struct f_tree *child;
        int state;
}f_tree;

typedef struct f_changefile{
        char fname[BUFFER_SIZE];
        time_t time;
        int state;
}f_changefile;

char path[BUFFER_SIZE];
char check_path[BUFFER_SIZE];
f_tree * make_tree(char *path);//학번디렉토리안의 파일들을 트리화하는 함수

//f_changefile f_change[BUFFER_SIZE];
//int iOption=0;
//int rOption=0;

void doDelete(char *token[]);
void prompt(void);
void print_usage(void);
void print_tree(f_tree *head,int depth);

void write_log(int num);
int w_createlist(f_tree *tree,int state,int index);
void initstat(f_tree *cur);
void sort_list(int num);
int w_changelist(f_tree *tree,int cnt);
void compare_tree(f_tree *cur,f_tree *prev);
int compare_node(f_tree *cur,f_tree *prev);
char log_path[BUFFER_SIZE];//log.txt의 절대경로
char check_path[BUFFER_SIZE];//check디렉토리의 절대경로

