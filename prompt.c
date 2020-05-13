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
int lOption=0;
int dOption=0;

void prompt(){
	char input[BUFFER_SIZE];
	char tpath[BUFFER_SIZE],spath[BUFFER_SIZE];
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

	printf("prompt parent : %d\n", pid);


	while(1){
		if(pid !=getpid())
			break;
		memset(token,0,MAX_TOKEN);//token을 다시 비워줌
		/*for(int i=0;i<MAX_TOKEN;i++)
		  memset(token[i],0,MAX_TOKEN);*/

		//프롬프트 모양 : "학번>"문자 출력
		printf("20182611>");

		//delete,size,recover,tree,exit,help수행
		//이외는 help실행과 동일 출력후 continue;
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
			if(strcmp(token[1],"")==0){
				fprintf(stderr,"input FILENAME\n");
				exit(1);
			}

			//-i,-r옵션 처리
			//if(argc==2 || argc==3 || argc==4){//delete뒤에 옵션없이  파일이름만 붙을경우
			if(argc>2){

				if(!strcmp(token[2],"-i") || !strcmp(token[4],"-i"))
					iOption=1;

				else if((strcmp(token[2],"-r")==0) || (strcmp(token[4],"-r"))==0)
					rOption=1;

			}
			doDelete(token);
			check_info();//info 디렉토리의 사이즈 체크
			continue;
		} else if(strcmp(token[0],"size")==0){
			int num=1;
			printf("\nsize 호출\n");
			chdir(check_path);

			if(access(token[1],F_OK)<0){
				fprintf(stderr,"access error for %s\n",token[1]);
				break;
			} else{

				if(argc==2){

				} else if(!strcmp(token[2],"-d")){
					dOption=1;
					num=atoi(token[3]);
					printf("num : %d\n",num);
				}
				realpath(token[1],spath);
				printf("%s\n",spath);
				if(strstr(spath,check_path)==NULL) {//해당 경로가 체크디렉토리에 없다면 에러처리
					fprintf(stderr,"file doesn't exist in checkpath\n");
					break;
				}
				head=make_tree(spath);
				print_size(head,spath,num,1);
				//free_tree(head);
				continue;
			}

		} else if(strcmp(token[0],"recover")==0) {
			/* 
			 * RECOVER 옵션 : recover [filename] [option]
			 * trash 디렉토리 안에 있는 파일을 원래 경로로 복구
			 * 동일한 이름이 있을 경우 파일이름, 삭제시간, 수정시간 보여줌
			 * 복구 시 이름 중복된다면 파일 처음에 "숫자_" 추가
			 */
			if(argc>3 || argc==1){
				fprintf(stderr,"wrong input\n");
				continue;
			}

			if(!strcmp(token[2],"-l"))
				lOption=1;

			char fname[BUFFER_SIZE];
			strcpy(fname,token[1]);

			//함수에 들어가기 전 파일 존재여부 확인
			sprintf(tpath,"%s/%s/%s",path,"trash/infos",fname);
			if(access(tpath,F_OK)<0){
				fprintf(stderr,"There is no '%s' in the 'trash' directory!\n",fname);
				continue;
			}

			recover_file(fname);
			continue;

		} else if(strcmp(token[0],"tree")==0){

			head=make_tree(check_path);//check 디렉토리 파일 목록 트리 생성
			printf("check\n");
			//생성한 트리를 출력 
			print_tree(head->child,1);//check 디렉토리의 첫째부터 차례로 출력,1: 트리의 깊이
			free_tree(head);
			printf("\n");
			continue;

		} else if(strcmp(token[0],"exit")==0)
			break;		
			else if(strcmp(token[0],"help")==0) {
				print_usage();
				continue;
			} else {
				//이외의 명령어 수행 시 자동으로 help를 실행시킨 것과 동일하게 출력
				printf("Invalid instruction !\n");
				print_usage();
				continue;
			}
		}
		exit(0);
}


void print_size(f_tree *head,char *spath,int n,int swit){
	char *rpath;
	//printf("?\n");
	while(n>0){
		if(head==NULL) break;
			//현재경로로부터 상대경로를 추출해서 rpath에 저장
		rpath=head->fname+strlen(path);
		printf("%d	%s\n",head->size,rpath);
		if(swit==1){
			if(dOption==1){
				if(!S_ISDIR(head->statbuf.st_mode))
					break;
			}
			else break;
		}
		swit=0;
		/*
		   print_size(head->child,spath,n-1,1);
		   swit=0;
		   }
		   else
		   print_size(head->child,spath,n,0);
		   }*/
		if(head->child !=NULL)
			print_size(head->child,spath,n-1,swit);
		head=head->sibling;
		
	}
}

void print_tree(f_tree *head,int depth){
	char *filename,*tmp;
	f_tree *now=malloc(sizeof(f_tree));
	now=head;
	while(1){
		tmp=now->fname;
		filename=strtok(tmp,"/");
		while((tmp=strtok(NULL,"/"))!=NULL)
			filename=tmp;//딱 파일이름만 불러옴
		if(now->sibling!=NULL){
			printf("├─");
		}
		else{
			printf("└─");
		}
		printf(" %s ",filename);
		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->child !=NULL){
				printf("\n");
				depth++;

				for(int i=1;i<depth;i++){
					printf("│   ");
				}
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
		else	break;
	}
}
void doDelete(char *token[20]){
	int cnt=0,i,t=0;
	FILE *fp;
	struct tm tm;
	time_t end,now;
	struct stat statbuf;//delete
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
	//ENDTIME이 입력된경우
	if(token[2][0]>='0' && token[2][0]<'9' && token[3][0]>='0' && token[3][0]<'9')
	{

		//시간 가져오기
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
		iOption=0;
		if(endOption==1){
			sleep(t);
		}
		stat(tpath,&statbuf);
		if(S_ISDIR(statbuf.st_mode)){
			remove_directory(tpath);
		}else
			remove(tpath);

	}
	else{
		move_trash(tpath,t);
	}
	//일반삭제는 trash 디렉토리로 이동



}
void move_trash(char *path, int t){
	char c;

	sleep(t);

	if(rOption==1){
		printf("Delete [y/n]? ");
		c=fgetc(stdin);
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
	sprintf(files,"%s/%s",path,"trash/files");
	sprintf(infos,"%s/%s",path,"trash/infos");
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
	/*if(chdir(check_path)<0){//check 디렉토리 파일로 이동
	  fprintf(stderr,"chdir error");
	  exit(1);
	  }*/
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
	head->size=make_size(head->child);
	return head;
}
int make_size(f_tree *head){
	int size=0;
	while(head!=NULL){
		size+=head->size;
		head=head->sibling;
	}
	return size;
}

void free_tree(f_tree *head)
{
	if(head->child != NULL)
		free_tree(head->child);
	if(head->sibling != NULL)
		free_tree(head->sibling);

	free(head);
}
void check_info(void){
	char info_path[BUFFER_SIZE];
	struct dirent **namelist;
	struct stat statbuf;
	int amount,count;
	f_tree *head;
	sprintf(info_path,"%s/%s",path,"trash/infos");
	chdir(info_path);
	//arrange_trash에서 삭제된 이후에도 2KB가 넘을 수 있기 때문에 while문을 돌려 
	//크기 체크를 해주면서 반복해야함
	while(1){
		amount=9;
		count=scandir(info_path,&namelist,NULL,alphasort);
		for(int i=0;i<count;i++){
			if(!strcmp(namelist[i]->d_name,".") || !strcmp(namelist[i]->d_name,".."))
				continue;
			stat(namelist[i]->d_name,&statbuf);
			amount+=statbuf.st_size;
		}

		if(amount>1000){
			//trash디렉토리 재정리해줘야됨
			arrange_trash(namelist,count);
		}
		else break;
		for(int i=0;i<count;i++)
			free(namelist[i]);
		free(namelist);
	}
	//	printf("trash size : %d\n",amount);

}
void arrange_trash(struct dirent **namelist,int count){
	char info_path[BUFFER_SIZE],file_path[BUFFER_SIZE];
	struct stat statbuf;
	time_t oldtime=-1;
	char oldfile[BUFFER_SIZE];
	int isFirst=1;
	sprintf(info_path,"%s/%s",path,"trash/infos");
	sprintf(file_path,"%s/%s",path,"trash/files");
	chdir(info_path);

	for(int i=0;i<count;i++){
		if(!strcmp(namelist[i]->d_name,".") || !strcmp(namelist[i]->d_name,".."))
			continue;
		stat(namelist[i]->d_name,&statbuf);
		if(isFirst==1){
			oldtime=statbuf.st_mtime;
			strcpy(oldfile,namelist[i]->d_name);
			isFirst=0;
		}
		if(oldtime>statbuf.st_mtime){
			strcpy(oldfile,namelist[i]->d_name);
			//printf("%s\n",oldfile);
			oldtime=statbuf.st_mtime;
		}

	}
	remove(oldfile); //info_path에서 가장 오래된 파일 삭제
	chdir(file_path);
	remove(oldfile);//file_path에서 가장 오래된 파일 삭제

}


void recover_file(char *fname){
	char tmp[BUFFER_SIZE],infofile[BUFFER_SIZE],fpath[BUFFER_SIZE],cpath[BUFFER_SIZE],info_path[BUFFER_SIZE],file_path[BUFFER_SIZE],overname[BUFFER_SIZE];
	struct dirent **namelist;
	struct stat statbuf;
	char name[20],*str,dirpath[BUFFER_SIZE];
	int count,num=0,cnt=0;
	int idx=1;//복원시 중복파일 처리를 위한 인덱스
	FILE *fp;
	//time_t mtime[]
	char **D,**M;

	sprintf(file_path,"%s/%s",path,"trash/files");
	sprintf(info_path,"%s/%s",path,"trash/infos");
	chdir(info_path);

	count=scandir(info_path,&namelist,NULL,alphasort);
	time_t mtime[count];
	D=malloc(sizeof(char*)*count);
	M=malloc(sizeof(char*)*count);
	for(int i=0;i<count;i++){
		D[i]=malloc(sizeof(char)*BUFFER_SIZE);
		M[i]=malloc(sizeof(char)*BUFFER_SIZE);
	}
	for(int i=0;i<count;i++){
		if(!strcmp(namelist[i]->d_name,".") || !strcmp(namelist[i]->d_name,".."))
			continue;
		if((fp=fopen(namelist[i]->d_name,"r"))==NULL){
			fprintf(stderr,"fopen error for %s\n",namelist[i]->d_name);
			return;
		}
		stat(namelist[i]->d_name,&statbuf);
		mtime[i]=statbuf.st_mtime;
		//infos에 있는 내용 읽어오는 부분
		fread(tmp,13,1,fp);
		fscanf(fp,"%s\n",tmp);//둘째 줄 버림

		fread(D[num],23,1,fp);//삭제시간 받아옴		
		fgetc(fp);//개행문제 버림
		fread(M[num],23,1,fp);//삭제시간 받아옴

		num++;
	}

	if(lOption==1){
		//printf("l옵션 실행중");
		lOption=0;
		int i,j;
		time_t t_tmp;
		struct dirent *d_tmp;
		char *d;
		d=malloc(sizeof(char)*count);
		//	printf("%d\n",count);
		for(i=0;i<count;i++){//수정시간 순으로 정렬
			if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")){
				continue;
			}

			for(j=i+1;j<count;j++){
				if(mtime[i]>mtime[j]){
					t_tmp=mtime[j];
					mtime[j]=mtime[i];
					mtime[i]=t_tmp;
					d_tmp=namelist[i];
					namelist[i]=namelist[j];
					namelist[j]=d_tmp;
				}
			}
		}
		j=1;
		for(i=0;i<count;i++){
			if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
				continue;

			fp=fopen(namelist[i]->d_name,"r");
			fread(tmp,13,1,fp);
			fscanf(fp,"%s\n",tmp);
			fgetc(fp);
			fread(d,23,1,fp);//삭제시간 받아옴		
			printf("%d. %s		%s",j++,namelist[i]->d_name,d);
		}


	}
	for(int i=0;i<count;i++){
		//복원하고자 하는 파일의 이름 탐색
		if(strcmp(namelist[i]->d_name,fname)==0){
			cnt++;
			chdir(info_path);
			sprintf(infofile,"%s/%s",info_path,namelist[i]->d_name);
			fp=fopen(namelist[i]->d_name,"r");
			//읽기 권한으로 파일 오픈(위에서 이미 검사했으므로 오픈에러처리는 안해도 됨)
			fread(tmp,13,1,fp);//첫째 줄 버림

			//복원할 파일의 위치를 받아오고, 동일 위치에 파일이 있는지 확인
			fscanf(fp,"%s\n",cpath);
			strcpy(name,namelist[i]->d_name);
			sprintf(tmp,"%s/%s",file_path,name);
			if(access(cpath,F_OK)==0){
				//이미 파일이 존재한다면 파일 이름의 처음에 "숫자_"를 추가
				sprintf(overname,"%d_%s",idx++,name);
				//files에 있는 파일을 그대로 check 디렉토리로 옮김
				strcpy(name,overname);
			}

			remove(infofile);
			chdir(file_path);

			sprintf(fpath,"%s/%s",check_path,name);
			rename(tmp,fpath);

		}
		if(cnt>1){
			char ch;
			for(int i=0;i<num;i++)
				printf("%d. %s  %s %s\n",i+1,name,D[i],M[i]);
			printf("Choose : ");
			ch=getchar();
			getchar();

		}
	}

}
