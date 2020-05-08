#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/times.h>
#include<time.h>

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

f_changefile f_change[BUFFER_SIZE];
void write_log(int num);
int w_createlist(f_tree *tree,int state,int index);
void initstat(f_tree *cur);
void sort_list(int num);
int w_changelist(f_tree *tree,int cnt);
void compare_tree(f_tree *cur,f_tree *prev);
int compare_node(f_tree *cur,f_tree *prev);
void print_usage(void);
void prompt(char *);

char log_path[BUFFER_SIZE];//log.txt의 절대경로
char check_path[BUFFER_SIZE];//check디렉토리의 절대경로

void ssu_mntr(int argc,char *argv[]);
f_tree * make_tree(char *path);//학번디렉토리안의 파일들을 트리화하는 함수
void ssu_mntr(int argc,char *argv[]){
	FILE *fp;
	char pwd[BUFFER_SIZE];
	//char log_path[BUFFER_SIZE];//log.txt의 절대경로

	getcwd(pwd,BUFFER_SIZE);//현재위치
//printf("%s\n",pwd);
	if(access("check",F_OK)!=0){//학번디렉토리의 존재여부 확인
		fprintf(stderr,"no directory\n");
		exit(1);//없으면 에러
	}
	//printf("%s",pwd);
	sprintf(log_path,"%s/%s",pwd,"log.txt");
	sprintf(check_path,"%s/%s",pwd,"check");//
	//printf("%s\n",log_path);
	//변경사항을 저장할 log.txt파일을 오픈(없으면 생성)
	if((fp=fopen(log_path,"w+"))==NULL){
		fprintf(stderr,"file open error!\n");
		exit(1);
	}
	fclose(fp);

	int isfirst=1;
	int num;
	f_tree *prev_tree;
	f_tree *cur_tree;
	while(1){
		cur_tree=make_tree(check_path);//check디렉토리에 대한 트리 생성
		initstat(cur_tree);//state초기화
		
		//첫번째 실행인 경우 새롭게 만든 cur_tree를 기존 트리로 설정
		if(isfirst==1){
			prev_tree=cur_tree;
			isfirst=0;
			continue;
		}
		compare_tree(cur_tree->child,prev_tree->child);//기존, 현재 트리를 비교
		num=w_createlist(cur_tree->child,DELETE,0);//f_change 구조체를 채우는 함수(DELETE,CREATE 따로 생성 후 log.txt에 한번에 시간순으로 정렬해서 넣을것임)
		printf("num : %d\n",num);
		num=w_createlist(cur_tree->child,CREATE,num);
		printf("num : %d\n",num);
		sort_list(num);//구조체 시간순 정렬
		write_log(num);
		for(int i=0;i<num;i++)
			printf("f_change[%d]=%d,%s\n",i,f_change[i].state,f_change[i].fname);
//		break;
		//로그에 기록을 끝낸상태
		//new_tree를 prev_tree로 옮겨주는 작업수행 (계속 트리를 갱신해주기 위해 필요)
		//init_tree(prev_tree);
		prev_tree=cur_tree;
		
		sleep(1);
	}
}
void write_log(int num){
	char *tmp,fname[BUFFER_SIZE];
	
	char timeform[BUFFER_SIZE];
	char logform[BUFFER_SIZE];
	FILE *fp;
	struct tm t;
	if((fp=fopen(log_path,"r+"))<0){//log.txt파일 오픈(읽기+쓰기)
		fprintf(stderr,"file open error\n");
		exit(1);
	}//파일 오픈 에러처리
	fseek(fp,0,SEEK_END);//오프셋을 맨 뒤로 설정
	for(int i=0;i<num;i++){
		tmp=strstr(f_change[i].fname,"check/");//check디렉토리 안 파일을 받아와서 tmp에 저장
		tmp+=6;//check/뒤의 파일이름을 가리키도록 함
		strcpy(fname,tmp);//순수 파일이름만을 fname에 집어넣음
		//printf("%s\n",fname);
		t=*localtime(&f_change[i].time);
		sprintf(timeform,"%.4d-%02d-%02d %02d:%02d:%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
		//printf("%s\n",timeform);
		
		switch(f_change[i].state){
			case MODIFY : 

				fprintf(fp,"[%s][%s_%s]\n",timeform,"modify",fname);
				break;
			case CREATE:
				fprintf(fp,"[%s][%s_%s]\n",timeform,"create",fname);
				break;
			case DELETE : 
				fprintf(fp,"[%s][%s_%s]\n",timeform,"delete",fname);
				break;
		}		
		
	}
	fclose(fp);
}
void sort_list(int num){//f_change 구조체를 시간순으로 정렬해주는 함수
	f_changefile tmp;
	for(int i=0;i<num;i++){
		for(int j=i+1;j<num;j++){
			if(f_change[i].time>f_change[j].time){
				tmp=f_change[i];
				f_change[i]=f_change[j];
				f_change[j]=tmp;
			}
		}
	}
}
void initstat(f_tree *cur){//파일구조체의 state를 초기화해주는 함수
	while(1){
		cur->state=N;//현재노드의 state를 N으로 설정(아직 비교되지 않았음을 의미)
		
		//노드가 디렉토리라면 자식여부 확인 후 재귀
		if(S_ISDIR(cur->statbuf.st_mode))
			if(cur->child!=NULL)
				initstat(cur->child);
		//형제가 있다면 형제로 이동 후 초기화
		if(cur->sibling!=NULL)
			cur=cur->sibling;
		else break;//없다면 그대로 종료
	}
}
int w_createlist(f_tree *tree,int state,int index){//파일상태변경여부를 구조체에 저장
	//f_tree *stree;
	//stree=tree;
	while(1){
		//printf("%d",tree->state);
		if(tree->state==MODIFY){//MODIFY상태인경우(파일이 수정된 경우)
			strcpy(f_change[index].fname,tree->fname);
			f_change[index].time=tree->statbuf.st_mtime;
			f_change[index].state=MODIFY;
			index++;
			break;
		}
		else if(tree->state==N){
			if(state==CREATE){//CREATE인 경우 구조체의 시간을 수정시간으로 저장
				strcpy(f_change[index].fname,tree->fname);
				f_change[index].time=tree->statbuf.st_mtime;
				f_change[index].state=CREATE;
				index++;
				break;
			}
			if(state==DELETE){//DELETE상태인경우 구조체에서 시간을 0으로 설정(나머지는 동일)
				strcpy(f_change[index].fname,tree->fname);
				f_change[index].time=time(NULL);
				f_change[index].state=DELETE;
				index++;
				break;
			}
		}
		if(S_ISDIR(tree->statbuf.st_mode))
			if(tree->child !=NULL)
				index=w_createlist(tree->child,state,index);
		if(tree->sibling !=NULL)
			tree=tree->sibling;
		else
			break;
	}
	return index;
		
	/*while(1){
		//switch(tree->status){
			
		
	}*/
}

void compare_tree(f_tree *cur,f_tree *prev){
	//f_tree *stat=cur;
	while(1){
		compare_node(cur,prev);//각 노드를 비교
		//경우의 수 : 현재노드의 값이 이전노드에 아예 없었던 경우(create)
		//현재노드값과 같은노드의 값이 있는데 시간이 수정된경우(modify)
		//이전노드에는 있었는데 현재노드엔 없는경우(delete)
		//
		if(S_ISDIR(prev->statbuf.st_mode)){//만약 prev의 노드값이 디렉토리라면
			if(prev->child !=NULL)//자식을 가지고 있다면 자식을 먼저 비교
				compare_tree(cur,prev->child);
		}
		if(prev->sibling!=NULL){//형제가 있다면 형제노드로 이동
			prev=prev->sibling;
		}else//형제 없으면 멈춤
			break;
		//if(!strcmp(cur->name,prev->name)){//노드의 이름이 같을 경우
		//	if(cur->statbuf.st_mtime !=prev->statbuf.st_mtime)//수정 시간이 변경된 경우 MODIFY로 상태 변경
		//		cur->state=MODIFY;
		//}
	///	else{
	//	}
		//노드 이름이 다른경우 delete아님?
		//
	}
}
int compare_node(f_tree *cur,f_tree *prev){
	while(1){
		if(strcmp(cur->fname,prev->fname)==0){//같은 이름을 가진 파일이 있는경우
			
			printf("%s랑 %s이름같앙\n",cur->fname,prev->fname);
			if(cur->statbuf.st_mtime != prev->statbuf.st_mtime)//수정시간이 다른경우
				cur->state=MODIFY;//트리의 상태를 MODIFY로 변경
			return 1;
		}
	
			printf("%s랑 %s이름 달랑\n",cur->fname,prev->fname);
			if(S_ISDIR(cur->statbuf.st_mode))
				if(cur->child !=NULL)
					if(compare_node(cur->child,prev)==1)
						break;
			if(cur->sibling!=NULL){
				printf("%s의 sibling(%s)있음\n",cur->fname,cur->sibling->fname);
				cur=cur->sibling;
			}
			else{
				printf("%s는 sibling없음\n",cur->fname);
				cur->state=DELETE;
				break;
			}
	
		//return 0;
	}
	return 0;
}
f_tree* make_tree(char *path){//파일 트리만들기 
	//struct dirent **items;
	int cnt;
	int isfirst=1;
	char tmp[BUFFER_SIZE];
	if(chdir(path)<0){//check 디렉토리 파일로 이동
		fprintf(stderr,"chdir error");
		exit(1);
	}
	f_tree *head=malloc(sizeof(f_tree));
	f_tree *now=malloc(sizeof(f_tree));
	now=head;
	strcpy(head->fname,path);
	stat(head->fname,&(head->statbuf));
	cnt=scandir(".",&(head->namelist),NULL,alphasort);//디렉토리 안에 있는 모든 파일 읽어옴
	//cnt=탐색할 파일의 개수
	for(int i=0;i<cnt;i++){
		f_tree *new=malloc(sizeof(f_tree));
		new->sibling=NULL;
		new->child=NULL;
		if((!strcmp(head->namelist[i]->d_name,".")) ||(!strcmp(head->namelist[i]->d_name,"..")))
			continue;
		strcpy(new->fname,head->namelist[i]->d_name);//새로운 대의 파일이름을 생성
		sprintf(tmp,"%s/%s",path,new->fname);
		//printf("%s\n",tmp);
		strcpy(new->fname,tmp);
		stat(tmp,&(new->statbuf));
		//lstat(items[i]->d_name,&fstat);
		if(S_ISDIR(new->statbuf.st_mode))//파일의 성격이 디렉토리라면 다시 트리를 만듦(재귀)
			new=make_tree(tmp);
		if(isfirst==1){
			now->child=new;
			now=now->child;
			isfirst=0;
		}
		else{
			now->sibling=new;
			now=now->sibling;
		}
		//char resource[BUFFER_SIZE];
		//sprintf(resource,"%s/%s",path,items[i]->d_name);
		//printf("%s\n",resource);//파일 내용 출력
		//struct stat src;
		//sprintf(
		//
	}
	return head;
}
void print_log(char *name,int index){
	time_t t=time(NULL);
	struct tm* ti=localtime(&t);
	
}
void prompt(char *ar){
	while(1){
		//프롬프트 모양 : "학번>"문자 출력
		printf("20182611>");

		//delete,size,recover,tree,exit,help수행
		//이외는 help실행과 동일 출력후 continue;
		char input[BUFFER_SIZE];
		char *token[MAX_TOKEN];//최대 입력 토큰을 20개로 가정
		fgets(input,BUFFER_SIZE,stdin);//명령어 읽어들이기
		
		//엔터 입력시 프롬프트 재출력
		if(strcmp(input,"\n")==0){
			continue;
		}

		token[0]=strtok(input," ");//첫번째 글자인 명령어를 가져옴
		int argc=1;
		while(argc<MAX_TOKEN){
			//공백문자를 기준으로 토큰을 분리
			token[argc]=strtok(NULL," ");

			if(token[argc]==NULL){//토큰에 아무값도 들어가지 않은 경우
				for(int i=0;;i++){

					if(token[argc-1][i]=='\n'){
					//토큰의 맨 마지막에 들어온 개행문자를 제거
						token[argc-1][i]='\0';
						break;
					}
				}
				break;
			}
			argc++;
		}
	
		//delete 옵션
		if(strcmp(token[0],"delete")==0){
			printf("\ndelete 호출\n");
			continue;
		}
		//size 옵션
		else if(strcmp(token[0],"size")==0){
			printf("\nsize 호출\n");
			continue;
		}
		//recover 옵션
		else if(strcmp(token[0],"recover")==0){
			printf("\nrecover 호출\n");
			continue;
		}
		//tree 옵션
		else if(strcmp(token[0],"tree")==0){
			printf("\ntree호출\n");
			continue;
		}
		//exit 옵션
		else if(strcmp(token[0],"exit")==0){
			break;
		}
		//help 옵션
		else if(strcmp(token[0],"help")==0){
			print_usage();
			continue;
		}
		//이외의 명령어 수행 시 자동으로 help를 실행시킨 것과 동일하게 출력
		else{
			printf("Invalid instruction !\n");
			print_usage();
			continue;
		}
	}
	exit(0);
}

void print_usage(void){
	printf("***************************************************\n");
	printf("[print usage]\n\n");
	printf("delete		delete file at specified time.");
        printf(" ex> delete [FILENAME][ENDTIME][OPTION] \n");
	printf("size		print file size, route.");
        printf(" ex> size [FILENAME][OPTION : -d]\n");
	printf("recover		recover files to original path.");
	printf(" ex> recover [FILENAME][OPTION : -l]\n");
	printf("tree 		print tree\n");
	printf("exit		end the program\n");
	printf("help		print usage\n");
	printf("***************************************************\n");
}
