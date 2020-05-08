#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define MAX_TOKEN 20 //입력가능한 최대 토큰 수를 20개로 가정
#define MODIFY 0
#define DELETE 1
#define CREATE 2
#define N -1 


void prompt(char *ar);
void print_usage(void);

