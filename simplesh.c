/* simplesh.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipe_A(char *temp, char *temp2, char *temp3);
int background(char *temp);
int redirect_A(char *temp, char *temp2, char *temp3);
void handler(int sig);
pid_t pid;


int main()
{
	char buf[256];
	char *argv[50];
	int narg;
	//pid_t pid;

	signal(SIGINT,handler);
        signal(SIGTSTP,handler);
	signal(SIGCHLD,handler);

	while (1) {

		printf("shell> ");
		gets(buf);
		narg = getargs(buf, argv);
		pid = fork();
		if (pid == 0){
			execvp(argv[0], argv);
		}
		else if (pid > 0){
			wait((int *) 0);
		}
		else
			perror("fork failed");
	}
}
int getargs(char *cmd, char **argv)
{
	int narg = 0;
	char *home = getenv("HOME");
	while (*cmd) {
		if (*cmd == ' ' || *cmd == '\t')
			*cmd++ = '\0';
		else {
			argv[narg++] = cmd++;
			while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
				cmd++;
		}

	}
	// exit 구현
	if(!strcmp(argv[0],"exit")){
		exit(1);
	}
	// cd 구현
	else if(!strcmp(argv[0],"cd")){
		if(argv[1]!=NULL){ // 경로가 있을 경우
			chdir(argv[1]);
		}
		else{ // 경로가 없을 경우 home으로 이동
			chdir(home);
		}
		system("pwd");
	}
	// 빈 문자열 처리
	else if(argv[0] == '\n')){
		break;
	}
	// 이외 명령어들 execvp로 전달 마지막인자 NULL 처리
	else {
		argv[narg] = NULL;
	}
	// 두번째 인자가 NULL이 아닐경우
	if(argv[1]!=NULL){
		// 파이프 구현
		if(!strcmp(argv[1],"|")){
			pipe_A(argv[0], argv[1], argv[2]);
			argv[0]=NULL;
			argv[1]=NULL;
			argv[2]=NULL;
		}
		// 백그라운드 구현
		else if(!strcmp(argv[1],"&")){
			background(argv);
			argv[0]=NULL;
			argv[1]=NULL;
		}
		// 리다이렉터 구현
		else if(!strcmp(argv[1],"<")||!strcmp(argv[1],">")){
			redirect_A(argv[0], argv[1], argv[2]);
			argv[0]=NULL;
			argv[1]=NULL;
			argv[2]=NULL;
		}
	}
	return narg;
}

/*
	백그라운드 구현
	ls
	인자1
*/
int background(char *temp){
	pid_t pid1;
	pid1 = fork();
	switch(pid1)
	{
		case -1:
			perror("error");
		case 0:
			temp[1] = NULL;
			signal(SIGINT,SIG_IGN);
			execvp(temp[0],temp);
			exit(1);
		default:
			printf("%d\n",getpid());
	}
}

/*
	파이프 구현
	ls     |    more
	ps     |    more
	인자1 인자2 인자3
*/
int pipe_A(char *temp, char *temp2, char *temp3){
	char *a[2];
	char *b[2];
	int fd[2];
	pid_t pid1, pid2;
	a[0]=temp;
	a[1]=NULL;

	b[0]=temp3;
	b[1]=NULL;
	

	if(pipe(fd)==-1)
	{
		perror("fork");
		return -1;
	}
	pid1=fork();
	switch(pid1)
	{
		case -1:
			perror("forked error");
		case 0:
			close(fd[0]);
			if(fd[1]!=1){
				dup2(fd[1],1);
				close(fd[1]);
			}
			execvp(a[0],a);
	}
	pid2=fork();
	switch(pid2)
	{
		case -1:
			perror("forked error");
		case 0:
			close(fd[1]);
			if(fd[0]!=0){
				dup2(fd[0],0);
				close(fd[0]);
			}
			execvp(b[0],b);
	}
	close(fd[0]);
	close(fd[1]);
	while(wait(NULL)!=-1);
	return 0;
}

/*
	리다이렉터 구현
	cat     >   test.txt
	cat     <   test.txt
	인자1  인자2   인자3
*/

int redirect_A(char *temp, char *temp2, char *temp3){
	int fd[2];
	int fd_o;
	char *a[2];
	char *b[2];
	int i=0;
	int j=0;
	int z=0;
	pid_t pid;
	int check_count = 0;

	a[0]=temp;
	a[1]=NULL;

	b[0]=temp3;
	b[1]=NULL;

	if(!strcmp(temp2,"<")){
		check_count=1;
	}
	else{
		check_count=2;
	}

	if(check_count==1){
		switch(fork()){
			case -1:
				perror("fork()");
				break;
			case 0:
				fd_o=open(b[0],O_RDONLY);
				if(fd_o==-1)
					perror("error");
				if(dup2(fd_o,STDIN_FILENO)==-1)
					perror("fd_o error");
				close(fd_o);
				execvp(a[0],a);
				break;
			default:
				wait(NULL);
		}
	}
	if(check_count==2){
		switch(fork()){
			case -1:
				perror("fork()");
				break;
			case 0:
				fd_o = open(b[0],O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if(fd_o==-1)
					perror("error filter\n");
				if(dup2(fd_o,1)==-1){
					perror("fd_o dur error\n");
				}
				close(fd_o);
				execvp(a[0],a);
				exit(0);
				break;
		}
		return 0;
	}

}

/*
	signal handler
	Ctrl+C, Ctrl+Z에 대한 작업 진행
*/
void handler(int sig){             //signal 핸들러
	int status;
	if(pid != 0){                //자식과 부모의 구분
		switch(sig){
			case SIGINT:
				printf("Ctrl + c SIGINT\n");
				break;
			case SIGTSTP:
				printf("Ctrl + z SIGTSTP\n");
				kill(0,SIGCHLD); //stpt 받았을 때, 자신은 다시 run
				break;
			case SIGCONT:
				printf("Restart rs SIGCONT\n");
				break;
			case SIGCHLD:
				waitpid(-1, &status, WUNTRACED);
				break;
                }
        }
}
