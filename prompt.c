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
int iOption=0;
int rOption=0;


void prompt(){
	char input[BUFFER_SIZE];
	char *token[MAX_TOKEN];
	memset(path,0,BUFFER_SIZE);
	memset(check_path,0,BUFFER_SIZE);

	getcwd(path,BUFFER_SIZE);//path : 현재디렉토리의 경로
	sprintf(check_path,"%s/%s",path,"check"); //check_path : check디렉토리의 경로
	
	f_tree *head=malloc(sizeof(f_tree));
	pid_t pid,daemon;
	pid=getpid();
	if((daemon=vfork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
	}
	else if(daemon==0){
		execl("./monitoring","",(char*)0);
	}


	while(1){
		if(pid !=getpid())
			break;
		//for(int j=0;j<20;j++
		memset(token,0,MAX_TOKEN);//token을 다시 비워줌

		//프롬프트 모양 : "학번>"문자 출력
		printf("20182611>");
		
		//delete,size,recover,tree,exit,help수행
		//이외는 help실행과 동일 출력후 continue;
		fgets(input,BUFFER_SIZE,stdin);//명령어 읽어들이기
		//token[strlen(token)-1]='\0';
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
				token[argc]="\0";
			break;
			}
			argc++;
		}
		/*
		delete 옵션 : DELETE [FILENAME] [END_TIME] [OPTION]
		지정한 삭제 시간에 자동으로 파일을 삭제해주는 명령어
		*/
		if(strcmp(token[0],"delete")==0){
			//입력된 옵션 확인
			//printf("\ndelete 호출\n");
			//FILENAME 입력이 없는 경우 에러처리
			if(strcmp(token[1],"")==0){
				fprintf(stderr,"input FILENAME\n");
				exit(1);
			}
			//-i,-r옵션 처리
			//if(argc==2 || argc==3 || argc==4){//delete뒤에 옵션없이  파일이름만 붙을경우
			//printf("무사히 넘김,%d\n",argc);
			//}
			if(!strcmp(token[2],"-i") || !strcmp(token[4],"-i")){
				printf("-i입력\n");
				iOption=1;
			}
			else if((strcmp(token[2],"-r")==0) || (strcmp(token[4],"-r"))==0){
				printf("-r입력\n");
				rOption=1;
			}

			doDelete(token);
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
		//TREE 옵션
		else if(strcmp(token[0],"tree")==0){
			head=make_tree(check_path);//check 디렉토리 파일 목록 트리 생성
			printf("check\n");
			//생성한 트리를 출력 
			print_tree(head->child,1);//check 디렉토리의 첫째부터 차례로 출력,1: 트리의 깊이
			free_tree(head);
			printf("\n");
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
//int isFirst=1;
void print_tree(f_tree *head,int depth){
	char *filename,*tmp;
	//int isFirst=1;
	f_tree *now=malloc(sizeof(f_tree));
	now=head;
	while(1){
		tmp=now->fname;
		filename=strtok(tmp,"/");
		while((tmp=strtok(NULL,"/"))!=NULL)
			filename=tmp;//딱 파일이름만 불러옴
	//	printf("filename : %s\n",filename);//확인용
		/*if(isFirst==1){
			printf("├─");
			isFirst=0;
		}
		else{*/
			//if(depth>
		if(now->sibling!=NULL)printf("├─");
		else printf("└─");
		
		printf(" %s ",filename);
		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->child !=NULL){
				printf("\n");
				depth++;
				for(int i=1;i<depth;i++)
					printf("│   ");
				
				print_tree(now->child,depth);
				depth--;
			}
		}
		if(now->sibling !=NULL){
			now=now->sibling;
			
			printf("\n");
			for(int i=1;i<depth;i++)
		              printf("│   ");
		}
		else
			break;
	}
	//printf("\n");	

}
void doDelete(char *token[20]){
	int cnt=0,i,t=0;
	FILE *fp;
	struct tm tm;
	time_t end,now;
	struct stat statbuf;//delete
	//iOption=0;
	//rOption=0;
	int endOption=0;
	char fname[BUFFER_SIZE];
	char trash_path[BUFFER_SIZE];
	char *tpath,*tmp;//삭제할 파일의 절대경로
	struct dirent **namelist;
	memset(trash_path,0,BUFFER_SIZE);

	chdir(check_path);//체크디렉토리로 이동
	tpath=malloc(sizeof(char) * BUFFER_SIZE);//tpath의 공간할당
	realpath(token[1],tpath);//파일의 상대경로를 절대경로로 변경
	

	//입력한 파일이 절대경로일수도 있음
	//'/'을 단위로 문자를 끊어서 최종 파일 이름(fname)만 읽어들이는 과정
	tmp=malloc(sizeof(char)*BUFFER_SIZE);//tmp 메모리 할당
	strcpy(fname,token[1]);
	strcpy(tmp,token[1]);
	tmp=strtok(token[1],"/");
	while((tmp=strtok(NULL,"/"))!=NULL)
		strcpy(fname,tmp);
	cnt=scandir(check_path,&namelist,NULL,alphasort);//check디렉토리에 존재하는 파일 조회
	for(i=0;i<cnt;i++){//삭제 파일이 check디렉토리에 있는지 검사
		if(strcmp(namelist[i]->d_name,fname)==0)
			break;
	}
	if(i==cnt){
		fprintf(stderr,"%s doesn't exist in check_dir\n",fname);
		exit(1);
	}
	//삭제한 파일은 제출할 소스코드 디렉토리 밑에 있는 "trash"디렉토리로 이동
	chdir(path);
	if(access("trash",F_OK)<0)
		mkdir("trash",0755);//없으면 생성
	sprintf(trash_path,"%s/%s",path,"trash");
	chdir(trash_path);
	//files,info 서브 디렉토리 생성
	mkdir("files",0755);
	mkdir("infos",0755);
	//printf("ㄱ");
	//
	printf("%s\n",tpath);
	//ENDTIME이 입력된경우
	if(token[2][0]>='0' && token[2][0]<'9' && token[3][0]>='0' && token[3][0]<'9')
	{
		
		//시간 가져오기
		iOption=0;
		rOption=0;
		endOption=1;
		int year,mon,day,hour,min,sec;
		sscanf(token[2],"%d-%d-%d",&year,&mon,&day);
		sscanf(token[3],"%d:%d:%d",&hour,&min,&sec);

		tm.tm_year=year-1900;
		tm.tm_mon=mon-1;
		tm.tm_mday=day;
		tm.tm_hour=hour;
		tm.tm_min=min;
		tm.tm_sec=sec;

		now=time(NULL);//현재 시간 받아오기
		end=mktime(&tm);//종료 시간 생성
		t=difftime(end,now);
		if(t<0){
			printf("invalid endtime\n");
			exit(1);
		}
	}
	// iOption : trash디렉토리 이동 없이 삭제
	if(iOption==1){
		printf("iOption 입력\n");
		printf("%s\n",tpath);
		
		stat(tpath,&statbuf);
		if(S_ISDIR(statbuf.st_mode)){
			remove_directory(tpath);
		}else
			remove(tpath);

	}
	// rOption : 지정 시간에 삭제 시 삭제여부 확인
	else if(rOption==1){
		printf("rOption 입력\n");
	}
	else{
		printf("%s,%s\n",tpath,fname);
		move_trash(tpath,t);
	}
	//일반삭제는 trash 디렉토리로 이동
	

	
}
void move_trash(char *path, int t){
	char c;
	sleep(t);
	if(rOption==1){
		printf("Delete [y/n]? ");
		c=getchar();
		if(c=='y')
			move_file(path);
			//파일 delete
	}
	else
		move_file(path);
		//파일 삭제
}
void move_file(char *fpath){
	char files[BUFFER_SIZE],infos[BUFFER_SIZE],orig[BUFFER_SIZE];
	char tmp[BUFFER_SIZE],dform[BUFFER_SIZE],mform[BUFFER_SIZE];
	char *fname;
	struct tm tm;
	time_t t;
	FILE *fp;
	struct stat statbuf;
	stat(fpath,&statbuf);//파일 삭제 전의 정보 불러옴
	
	memset(tmp,0,BUFFER_SIZE);
	memset(dform,0,BUFFER_SIZE);
	memset(mform,0,BUFFER_SIZE);
	memset(orig,0,BUFFER_SIZE);
	strcpy(orig,fpath);
	fname=strtok(fpath,"/");
	while((fpath=strtok(NULL,"/"))!=NULL)
		strcpy(fname,fpath);
	//printf("%s",fname);//확인용
	sprintf(files,"%s/%s",path,"trash/files");
	sprintf(infos,"%s/%s",path,"trash/infos");
	printf("orig: %s\n",orig);
	printf("files : %s\n",files);
	sprintf(tmp,"%s/%s",files,fname);
	rename(orig,tmp);//trash/files로 해당 파일 이동
	t=time(NULL);//t : 삭제 시간
	tm=*localtime(&t);
	sprintf(dform,"D : %04d-%02d-%02d %02d:%02d:%02d",tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	sprintf(mform,"M : %04d-%02d-%02d %02d:%02d:%02d",tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec);

	chdir(infos);
	if((fp=fopen(fname,"w+"))<0){
		fprintf(stderr,"fopen error for %s\n",fname);
		exit(1);
	}
	fprintf(fp,"[Trash info]\n");
	fprintf(fp,"%s\n",orig);
	fprintf(fp,"%s\n",dform);
	fprintf(fp,"%s\n",mform);
	fclose(fp);
	chdir(check_path);
}
void remove_directory(const char *tpath){
	DIR *dp;
	char tmp[BUFFER_SIZE];
	struct dirent *dirp;
	struct stat statbuf;
	dp=opendir(tpath);
	while((dirp=readdir(dp))!=NULL){
		if(!strcmp(dirp->d_name,".") || !strcmp(dirp->d_name,".."))
			continue;
		sprintf(tmp,"%s/%s",tpath,dirp->d_name);
		lstat(tmp,&statbuf);
		if(S_ISDIR(statbuf.st_mode))
			remove_directory(tmp);
		else
			remove(tmp);
	}
	closedir(dp);
	rmdir(tpath);
}
void print_usage(){
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
	cnt=scandir(head->fname,&(head->namelist),NULL,alphasort);//디렉토리 안에 있는 >모든 파일 읽어옴
	//cnt=탐색할 파일의 개수
	//printf("%d\n",cnt);
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
		
	}
	return head;
}

void free_tree(f_tree *head)
{
	if(head->child != NULL)
		free_tree(head->child);
	if(head->sibling != NULL)
		free_tree(head->sibling);

	free(head);
}
