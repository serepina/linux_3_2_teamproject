#include <gtk/gtk.h> 

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
}

G_MODULE_EXPORT void on_bconnect_clicked(GtkButton *button, Data *data) {
	//gtk_label_set_text(GTK_LABEL(data->labeltitle), "CONNECT가 클릭됨!");
	const char *eport = gtk_entry_get_text(GTK_ENTRY(data->eport));
	const char *eip = gtk_entry_get_text(GTK_ENTRY(data->eip));
	const char *ename = gtk_entry_get_text(GTK_ENTRY(data->ename));
	printf("%s %s %s\n",eport,eip,ename);
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
