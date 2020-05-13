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
#include "monitoring.h"

f_changefile f_change[BUFFER_SIZE];
void set_daemon_process(void);
int main(int argc,char *argv[]){
	FILE *fp;
	char pwd[BUFFER_SIZE];
	set_daemon_process();
	getcwd(pwd,BUFFER_SIZE);//현재위치
	if(access("check",F_OK)!=0){//학번디렉토리의 존재여부 확인
		fprintf(stderr,"no directory\n");
		exit(1);//없으면 에러
	}
	sprintf(log_path,"%s/%s",pwd,"log.txt");
	sprintf(check_path,"%s/%s",pwd,"check");//
	//변경사항을 저장할 log.txt파일을 오픈(없으면 생성)
	if((fp=fopen(log_path,"w+"))==NULL){
		fprintf(stderr,"file open error!\n");
		exit(1);
	}
	fclose(fp);
	init_daemon();
	int isfirst=1;
	int num;
	f_tree *prev_tree;
	f_tree *cur_tree;

	while(1){
		num=0;
		cur_tree=make_tree(check_path);//check디렉토리에 대한 트리 생성
		initstat(cur_tree->child);//state초기화

		//첫번째 실행인 경우 새롭게 만든 cur_tree를 기존 트리로 설정
		if(isfirst==1){
			prev_tree=cur_tree;
			isfirst=0;
			continue;
		}

		compare_tree(cur_tree->child,prev_tree->child);//기존, 현재 트리를 비교

		num=w_createlist(prev_tree->child,DELETE,0);//f_change 구조체를 채우는 함수(DELETE,CREATE 따로 생성 후 log.txt에 한번에 시간순으로 정렬해서 넣을것임)
		num=w_createlist(cur_tree->child,CREATE,num);
		sort_list(num);//구조체 시간순 정렬
		write_log(num);


		//로그에 기록을 끝낸상태
		//new_tree를 prev_tree로 옮겨주는 작업수행 (계속 트리를 갱신해주기 위해 필요)
		prev_tree=cur_tree;

		initstat(prev_tree->child);

		sleep(1);
	}
}
void set_daemon_process(void){
	pid_t pid;
	if((pid=fork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
	}
	else if(pid!=0){//부모면 죽임
		exit(0);
	}
	setsid();//새 세션?생성

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
		t=*localtime(&f_change[i].time);
		sprintf(timeform,"%.4d-%02d-%02d %02d:%02d:%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);

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
		if(cur==NULL)
			break;
		cur->state=N;
		if(cur->child !=NULL)
			initstat(cur->child);
		cur=cur->sibling;
	}
}

int w_createlist(f_tree *tree,int state,int index){//파일상태변경여부를 구조체에 저장
	
	while(1){

		if(tree->state==MODIFY){//MODIFY상태인경우(파일이 수정된 경우)
			strcpy(f_change[index].fname,tree->fname);
			f_change[index].time=tree->statbuf.st_mtime;
			f_change[index].state=MODIFY;
			index++;
			break;
		}

		else if(tree->state==N){
			strcpy(f_change[index].fname,tree->fname);
			f_change[index].time=time(NULL);
			f_change[index].state=state;
			index++;
			break;
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
}

void compare_tree(f_tree *cur,f_tree *prev){
	
	while(1){
		compare_node(cur,prev);//각 노드를 비교

		if(S_ISDIR(prev->statbuf.st_mode)){//만약 prev의 노드값이 디렉토리라면
			if(prev->child !=NULL)//자식을 가지고 있다면 자식을 먼저 비교
				compare_tree(cur,prev->child);
		}
		if(prev->sibling!=NULL){//형제가 있다면 형제노드로 이동
			prev=prev->sibling;
		}else//형제 없으면 멈춤
			break;
	}
}

int compare_node(f_tree *cur,f_tree *prev){

	while(1){

		if(cur == NULL)
			break;

		if(strcmp(cur->fname,prev->fname)==0){//같은 이름을 가진 파일이 있는경우
			cur->state = CHECKED;

			if(cur->statbuf.st_mtime != prev->statbuf.st_mtime)//수정시간이 다른경우
				cur->state=MODIFY;//트리의 상태를 MODIFY로 변경

			prev->state = CHECKED;
			return 1;
		}

		if(cur->child !=NULL)
			if(compare_node(cur->child,prev)==1)
				break;

		cur=cur->sibling;

	}
	return 0;
}

f_tree* make_node(void){
	f_tree *tmp=calloc(1,sizeof(f_tree));
	memset(tmp->fname,0,BUFFER_SIZE);
	tmp->sibling=NULL;
	tmp->child=NULL;
	tmp->namelist=NULL;
	tmp->state=N;
	tmp->size=0;
	return tmp;
}

f_tree* make_tree(char *path){//파일 트리만들기 
	int cnt;
	int isfirst=1;
	char tmp[BUFFER_SIZE];
	if(chdir(path)<0){//check 디렉토리 파일로 이동
		fprintf(stderr,"chdir error");
		exit(1);
	}
	f_tree *head;
	f_tree *now;
	head=make_node();
	now=head;
	strcpy(head->fname,path);
	stat(head->fname,&(head->statbuf));
	cnt=scandir(".",&(head->namelist),NULL,alphasort);//디렉토리 안에 있는 모든 파일 읽어옴
	//cnt=탐색할 파일의 개수
	for(int i=0;i<cnt;i++){
		f_tree *new=make_node();
		if((!strcmp(head->namelist[i]->d_name,".")) ||(!strcmp(head->namelist[i]->d_name,"..")))
			continue;
		strcpy(new->fname,head->namelist[i]->d_name);//새로운 대의 파일이름을 생성
		sprintf(tmp,"%s/%s",path,new->fname);
		strcpy(new->fname,tmp);
		stat(tmp,&(new->statbuf));
		if(S_ISDIR(new->statbuf.st_mode))//파일의 성격이 디렉토리라면 다시 트리를 만듦(재귀)
			new=make_tree(tmp);
		else
			new->size=new->statbuf.st_size;
		if(isfirst==1){
			now->child=new;
			now=now->child;
			isfirst=0;
		}
		else{
			now->sibling=new;
			now=now->sibling;
		}
	}
	return head;
}
void init_daemon(void){
	pid_t pid;
	if((pid=fork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
	}else if(pid!=0){
		exit(0);
	}
	setsid();
}


void free_tree(f_tree *head)
{
	if(head->child != NULL)
		free_tree(head->child);
	if(head->sibling != NULL)
		free_tree(head->sibling);
	free(head->namelist);
	free(head);
}
