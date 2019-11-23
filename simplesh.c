/* simplesh.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>

int pipe_A(char *temp, char *temp2, char *temp3);
int background(char *temp);

int main()
{
	char buf[256];
	char *argv[50];
	int narg;
	pid_t pid;
	//sigset_t blockset;
	//sigemptyset(&blockset);
	//sigaddset(&blockset,SIGINT);
	//sigaddset(&blockset,SIGQUIT);
	//sigprocmask(SIG_BLOCK,&blockset,NULL);

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
