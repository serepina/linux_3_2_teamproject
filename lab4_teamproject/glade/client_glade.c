#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXLINE     511
#define NAME_LEN    20

char *EXIT_STRING = "exit";
char *FILE_STRING = "get";
char bufname[NAME_LEN];	// 이름
char bufmsg[MAXLINE];	// 메시지부분
char bufall[MAXLINE + NAME_LEN];
int maxsocket1;	// 최대 소켓번호 + 1
int s;		// 소켓
int namelen;	// 이름의 길이
fd_set read_fds;
time_t ct;
struct tm tm;
int count=0;
char filename[MAXLINE]; // ＊ 추가 ＊
char temp[20];		// ＊ 추가 ＊
char buf[100];		// ＊ 추가 ＊
int size;		// ＊ 추가 ＊
int filehandle;		// ＊ 추가 ＊
char *f;		// ＊ 추가 ＊
// 소켓 생성 및 서버 연결, 생성된 소켓리턴
int tcp_connect(int af, char *servip, unsigned short port);
void errquit(char *mesg) { perror(mesg); exit(1); }


typedef struct _Data Data;
struct _Data {
	GtkWidget *window;
	GtkWidget *bconnect;
	GtkWidget *bsend;
	GtkWidget *eport;
	GtkWidget *eip;
	GtkWidget *ename;
	GtkWidget *econtent;
	GtkWidget *labeltitle;
	GtkWidget *textview1;            
	GtkTextBuffer *textbuffer1;
}; 

// entry set: gtk_entry_set_text(GTK_ENTRY(data->ResultEntry),result_stringadd);
// entry get: const char *entrytemp=gtk_entry_get_text(GTK_ENTRY(data->ResultEntry));
// label set: gtk_label_set_text(GTK_LABEL(data->label), "CONNECT가 클릭됨!");

G_MODULE_EXPORT void quit(GtkWidget *window, gpointer data) {
	gtk_main_quit ();
}

G_MODULE_EXPORT void on_bsend_clicked(GtkButton *button, Data *data) {
	//gtk_label_set_text(GTK_LABEL(data->labeltitle), "SEND가 클릭됨!");
	const char *econtent = gtk_entry_get_text(GTK_ENTRY(data->econtent));
	printf("%s \n",econtent);
	gtk_text_buffer_set_text(data->textbuffer1, econtent, -1);
}

G_MODULE_EXPORT void on_bconnect_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labeltitle), "서버에 접속되었습니다.");
	const char *eport = gtk_entry_get_text(GTK_ENTRY(data->eport));
	const char *eip = gtk_entry_get_text(GTK_ENTRY(data->eip));
	const char *ename = gtk_entry_get_text(GTK_ENTRY(data->ename));
	s = tcp_connect(AF_INET, eip, eport); // 소켓 연결 요청 수행
	maxsocket1 = s + 1;
	FD_ZERO(&read_fds); // read_fds의 모든 소켓을 0으로 초기화
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
	if (!gtk_builder_add_from_file (builder, "Client.xml", &error)) {
		g_print ("UI 파일을 읽을 때 오류 발생!\n");
		g_print ("메시지: %s\n", error->message);
		g_free (error);
		return (1);
	} 
	data = g_slice_new (Data);
	data->window = GTK_WIDGET(gtk_builder_get_object(builder, "window_client"));
	data->bconnect = GTK_WIDGET(gtk_builder_get_object(builder, "bconnect"));
	data->bsend = GTK_WIDGET(gtk_builder_get_object(builder, "bsend"));
	data->eport = GTK_WIDGET(gtk_builder_get_object (builder, "eport"));
	data->eip = GTK_WIDGET(gtk_builder_get_object (builder, "eip"));
	data->ename = GTK_WIDGET(gtk_builder_get_object (builder, "ename"));
	data->econtent = GTK_WIDGET(gtk_builder_get_object (builder, "econtent"));
	data->labeltitle = GTK_WIDGET(gtk_builder_get_object (builder, "labeltitle"));
	data->textview1 = GTK_WIDGET(gtk_builder_get_object(builder, "textview1"));
	data->textbuffer1 = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "textbuffer1"));
	
	//input_ip = gtk_entry_get_text(eip);
	//input_port = gtk_entry_get_text(eport);
	//input_name = gtk_entry_get_text(ename);
	//input_connect = gtk_entry_get_text(econtent);

	//input1_int = atoi(input_ip); // String to Integer Conversion
	//input2_int = atoi(input_port);

	gtk_builder_connect_signals (builder, data);
	g_object_unref(G_OBJECT(builder));
	gtk_widget_show_all(data->window);
	gtk_main ();
	g_slice_free (Data, data);
	return (0);
}

int tcp_connect(int af, char *servip, unsigned short port) {
	struct sockaddr_in servaddr;
	int  s;
	// 소켓 생성
	if ((s = socket(af, SOCK_STREAM, 0)) < 0)
		return -1;
	// 채팅 서버의 소켓주소 구조체 servaddr 초기화
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	// 연결요청
	if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr))
		< 0)
		return -1;
	return s;
}
