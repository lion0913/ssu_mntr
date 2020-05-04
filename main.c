#include "ssu_mntr.h"

int main(int argc,char *argv[]){//메인함수
	pid_t pid;
	if((pid=fork())==0)//자식 프로세스일 경우 모니터링을 실행
		ssu_mntr(argc, argv);
	else if(pid>0)//부모프로세스인 경우 프롬프트를 실행함
		prompt(argv[1]);
	else{//fork에 실패할 경우(-1)
		fprintf(stderr,"fork error\n");
		exit(1);
	}
	exit(0);
}
