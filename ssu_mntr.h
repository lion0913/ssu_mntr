
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

void print_usage(void);//프로그램의 사용법을 알려주는 함수
void ssu_mntr(int argc, char *argv[]);//모니터링 함수(log.txt작성)
void prompt(char *ar);//프롬프트를 관리하는 함수
void print_log(char *name,int index);
