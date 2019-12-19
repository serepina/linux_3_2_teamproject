#include <gtk/gtk.h>

//------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define MAXLINE 511
#define MAX_SOCK 1024

void *thread_function(void *arg);
void *thread_function2(void *arg);
void addClient(int s, struct sockaddr_in *newcliaddr);
void removeClient(int s);
int getmax();
int  tcp_listen(int host, int port, int backlog);
void errquit(char *mesg) { perror(mesg); exit(1); }

char *EXIT_STRING = "exit"; // client 종료 요청 문자열
char *START_STRING = "Connected to chat_server \n"; // client 접속 시 알림 메시지
char *FILE_STRING = "get";
int maxsocket1;  // 최대 소켓번호 + 1
int client_num; // 채팅 참가자 수
int chat_num;   // 총 채팅 개수
int csocket_list[MAX_SOCK];   // client socket 목록
char ip_list[MAX_SOCK][20];  // 접속한 ip 목록
int slisten_socket; // server listen socket
char filename[20];	// ＊ 추가 ＊
int filehandle;		// ＊ 추가 ＊
struct stat obj;	// ＊ 추가 ＊
int size;		// ＊ 추가 ＊

time_t ct;
struct tm tm;

struct sockaddr_in caddr;
char buf[MAXLINE + 1]; // client에서 받은 메시지
int i, j, nbyte, accp_socket;
int addrlen = sizeof(struct sockaddr_in);
fd_set read_fds;    // 읽기를 감지할 fd_set 구조체
pthread_t a_thread;
pthread_t b_thread;

//------------------------------------------------------

typedef struct _Data Data;
struct _Data {
	GtkWidget *window;
	GtkWidget *bconnect;
	GtkWidget *bheip;
	GtkWidget *bip_list;
	GtkWidget *buser_num;
	GtkWidget *eport;
	GtkWidget *labelmenu;
	GtkWidget *textview1;            
	GtkTextBuffer *textbuffer1;
};

G_MODULE_EXPORT void quit(GtkWidget *window, gpointer data) {
	gtk_main_quit ();
}

// BUTTON CONNECT
G_MODULE_EXPORT void on_bconnect_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "CONNECT가 클릭됨!");
	const char *eport = gtk_entry_get_text(GTK_ENTRY(data->eport));

	slisten_socket = tcp_listen(INADDR_ANY, atoi(eport),5);

	pthread_create(&a_thread, NULL, thread_function, (void *)NULL);
	pthread_create(&b_thread, NULL, thread_function2, (void *)NULL);
}

// BUTTON HELP
G_MODULE_EXPORT void on_bhelp_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "HELP가 클릭됨!");
	gtk_text_buffer_set_text(data->textbuffer1, "client_num, chat_num, ip_list", -1);
}

// BUTTON IP LIST
G_MODULE_EXPORT void on_bip_list_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "IP LIST가 클릭됨!");
	char temp[100] = ""; 
	char temp2[100] = "";
	for(int i=0;i<client_num;i++){
		strcat(temp,ip_list[i]);
		sprintf(temp2, " %d", csocket_list[i]);
		strcat(temp,temp2);
		strcat(temp,"\n");
	}

	gtk_text_buffer_set_text(data->textbuffer1, temp, -1);
}

// BUTTON USER NUMBER
G_MODULE_EXPORT void on_buser_num_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "USER NUMBER가 클릭됨!");
	char temp[100] = "현재 참가자 수 = "; 
	char temp2[100] = "";
	sprintf(temp2, "%d", client_num);
	strcat(temp,temp2);
	gtk_text_buffer_set_text(data->textbuffer1, temp, -1);
}

// BUTTON CHAT NUMBER
G_MODULE_EXPORT void on_bchat_num_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "CHAT NUMBER가 클릭됨!");
	char temp[100] = "지금까지 오간 대화의 수 = "; 
	char temp2[100] = "";
	sprintf(temp2, "%d", chat_num);
	strcat(temp,temp2);
	gtk_text_buffer_set_text(data->textbuffer1, temp, -1);
}

int main (int argc, char *argv[]) { 
	GtkBuilder *builder;
	GError *error = NULL;
	Data *data; 
	gtk_init (&argc, &argv);
	const char *input_ip = (const char *)malloc(20);
	const char *input_port = (const char *)malloc(20);
	const char *input_name = (const char *)malloc(20);
	const char *input_connect = (const char *)malloc(20);

	/* 빌더 생성 및 UI 파일 열기 */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, "Server.xml", &error)) {
		g_print ("UI 파일을 읽을 때 오류 발생!\n");
		g_print ("메시지: %s\n", error->message);
		g_free (error);
		return (1);
	}

	data = g_slice_new (Data);
	data->window = GTK_WIDGET(gtk_builder_get_object(builder, "window_server"));
	data->bconnect = GTK_WIDGET(gtk_builder_get_object(builder, "bconnect"));
	data->bheip = GTK_WIDGET(gtk_builder_get_object(builder, "bheip"));
	data->bip_list = GTK_WIDGET(gtk_builder_get_object (builder, "bip_list"));
	data->buser_num = GTK_WIDGET(gtk_builder_get_object (builder, "buser_num"));
	data->eport = GTK_WIDGET(gtk_builder_get_object (builder, "eport"));
	data->labelmenu = GTK_WIDGET(gtk_builder_get_object (builder, "labelmenu"));
	data->textview1 = GTK_WIDGET(gtk_builder_get_object(builder, "textview1"));
	data->textbuffer1 = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "textbuffer1"));

	gtk_builder_connect_signals (builder, data);
	g_object_unref(G_OBJECT(builder));
	gtk_widget_show_all(data->window);
	gtk_main ();
	g_slice_free (Data, data);

	return (0);
}

void *thread_function(void *arg) { //명령어를 처리할 스레드
	int i;
	printf("명령어 목록 : help, client_num, chat_num, ip_list\n");
	while (1) {
		char bufmsg[MAXLINE + 1];
		fprintf(stderr, "\033[1;32m");		//글자색을 녹색으로 변경
		printf("server>"); 			//커서 출력
		fgets(bufmsg, MAXLINE, stdin); 		//명령어 입력
		if (!strcmp(bufmsg, "\n")) continue;   	//엔터 무시
		else if (!strcmp(bufmsg, "help\n"))    	//명령어 처리
			printf("help, client_num, chat_num, ip_list\n");
		else if (!strcmp(bufmsg, "client_num\n"))	//명령어 처리
			printf("현재 참가자 수 = %d\n", client_num);
		else if (!strcmp(bufmsg, "chat_num\n"))		//명령어 처리
			printf("지금까지 오간 대화의 수 = %d\n", chat_num);
		else if (!strcmp(bufmsg, "ip_list\n")) 		//명령어 처리
			for (i = 0; i < client_num; i++)
				printf("%s %d\n", ip_list[i],csocket_list[i]);	// ＊ 추가 ＊
		else //예외 처리
			printf("해당 명령어가 없습니다.help를 참조하세요.\n");
	}
}

void *thread_function2(void *arg) {

	while(1){
		FD_ZERO(&read_fds); // read_fds의 모든 소켓을 0으로 초기화
		FD_SET(slisten_socket, &read_fds);  // I/O 변화를 감지할 초기 소켓 선택
		for(int i = 0; i < client_num; i++){    // 모든 client 접속 소켓 선택
			FD_SET(csocket_list[i], &read_fds);
		}
		
		maxsocket1 = getmax() + 1; // 최대 소켓 번호 크기 + 1 재 계산
		if(select(maxsocket1, &read_fds, NULL, NULL, NULL) < 0){
			errquit("select fail");
		}
		
		if(FD_ISSET(slisten_socket, &read_fds)) {
			accp_socket = accept(slisten_socket, (struct sockaddr*)&caddr,&addrlen);
			if(accp_socket == -1) {
				errquit("accept fail");
			}
			addClient(accp_socket, &caddr);
			send(accp_socket, START_STRING, strlen(START_STRING), 0); // 소켓으로 데이터 전송
			ct = time(NULL);
			tm = *localtime(&ct);
			write(1, "\033[0G", 4);	 	// 커서의 x 좌표를 0으로 이동
			printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
			fprintf(stderr, "\033[33m");    	// 글자색을 노란색으로 변경
			printf("사용자 1명 추가. 현재 참가자 수 = %d\n", client_num);
			fprintf(stderr, "\033[32m");    	// 글자색을 녹색으로 변경
			fprintf(stderr, "server>");     	// 커서 출력
		}
	
		// 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송
		for(int i=0; i < client_num; i++) {
			if(FD_ISSET(csocket_list[i], &read_fds)) {
				chat_num++; // 총 대화수 증가
				nbyte = recv(csocket_list[i], buf, MAXLINE, 0); // 소켓으로 부터 데이터 읽음
				if(nbyte <= 0) {
					removeClient(i);    // 클라이언트의 종료
					continue;
				}
				buf[nbyte] = 0;
				if(strstr(buf, EXIT_STRING) != NULL){ // 종료 문자 처리
					removeClient(i);    // 클라이언트의 종료
					continue;
				}
				if(strstr(buf, FILE_STRING) != NULL){				// ＊ 추가 ＊
					sscanf(buf, "%s%s", filename, filename);		// ＊ 추가 ＊
					stat(filename, &obj);					// ＊ 추가 ＊
					filehandle = open(filename, O_RDONLY);			// ＊ 추가 ＊
					size = obj.st_size;					// ＊ 추가 ＊
					if (filehandle == -1)					// ＊ 추가 ＊
						size = 0;					// ＊ 추가 ＊
					send(accp_socket, &size, sizeof(int), 0);		// ＊ 추가 ＊
					if (size)						// ＊ 추가 ＊
						sendfile(accp_socket, filehandle, NULL, size);	// ＊ 추가 ＊
					send(csocket_list[i], buf, nbyte, 0);			// ＊ 추가 ＊
					break;							// ＊ 추가 ＊
			
				}
		
				// 모든 채팅 참가자에게 메시지 방송
				for (j = 0; j < client_num; j++){
					send(csocket_list[j], buf, nbyte, 0);
				}
			}
		}
	}
}

// 새로운 채팅 참가자 처리
void addClient(int s, struct sockaddr_in *newcliaddr) {
	char buf[20];
	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf)); // ipv4와 ipv6 주소를 binary 형태에서 사람이 알아보기 쉬운 텍스트 형태로 전환해줌
	write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m");	//글자색을 노란색으로 변경
	printf("new client: %s\n", buf);//ip출력
	// 채팅 클라이언트 목록에 추가
	csocket_list[client_num] = s;
	strcpy(ip_list[client_num], buf);
	client_num++; //유저 수 증가
}

// 채팅 탈퇴 처리
void removeClient(int s) {
	close(csocket_list[s]);
	if (s != client_num - 1) { 		//저장된 리스트 재배열
		csocket_list[s] = csocket_list[client_num - 1];
		strcpy(ip_list[s], ip_list[client_num - 1]);
	}
	client_num--; 				//유저 수 감소
	ct = time(NULL);			//현재 시간을 받아옴
	tm = *localtime(&ct);
	write(1, "\033[0G", 4);			//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m");		//글자색을 노란색으로 변경
	printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("채팅 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", client_num);
	fprintf(stderr, "\033[32m");		//글자색을 녹색으로 변경
	fprintf(stderr, "server>"); 		//커서 출력
}

// 최대 소켓번호 찾기
int getmax() {
	// Minimum 소켓번호는 가정 먼저 생성된 slisten_socket
	int max = slisten_socket;
	int i;
	for (i = 0; i < client_num; i++)
		if (csocket_list[i] > max)
			max = csocket_list[i];
	return max;
}

// listen 소켓 생성 및 listen
int tcp_listen(int host, int port, int backlog) {
	int sd;
	struct sockaddr_in servaddr;

	sd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sd == -1) {
		perror("socket fail");
		exit(1);
	}
	// servaddr 구조체의 내용 세팅
	bzero((char *)&servaddr, sizeof(servaddr)); // 원하는 메모리 영역을 '0'으로 초기화 (초기화를 수행할 메모리 영역의 시작주소, 시작 주소로부터 초기화를 수행할 크기)
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind fail");  exit(1);
	}
	// 클라이언트로부터 연결요청을 기다림
	listen(sd, backlog);
	return sd;
}
