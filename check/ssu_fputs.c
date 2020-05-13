#include<stdio.h>
#include<stdlib.h>
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	char buf[BUFFER_SIZE];
	FILE *fp;
	if(argc!=2){//인자를 제대로 받지 못한 경우 에러처리
		fprintf(stderr,"usage : %s <file>\n",argv[0]);
		exit(1);
	}
	if((fp=fopen(argv[1],"w+"))==NULL){//파일 열기(쓰기 전용)
		fprintf(stderr,"fopen error for %s\n",argv[1]);
		exit(1);
	}//파일오픈에러처리
	fputs("Input String >>",stdout);//표준출력으로 출력
	gets(buf);//버퍼에 내용 입력받음
	fputs(buf,fp);//내용을 파일에 씀
	rewind(fp);//파일 오프셋 처음으로 이동
	fgets(buf,sizeof(buf),fp);//파일에서 버퍼만큼을 읽음
	puts(buf);//표준출력에 출력
	fclose(fp);
	exit(0);
}
