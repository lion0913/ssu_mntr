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
#include "prompt.h"
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
		pid_t daemon;
		if((daemon=vfork())<0){
			fprintf(stderr,"fork error\n");
			exit(1);
		}
		else if(daemon==0){
			execl("./monitoring","",(char*)0);
		}
		
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
