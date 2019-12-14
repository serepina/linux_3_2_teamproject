#include <gtk/gtk.h> 

typedef struct _Data Data;
struct _Data {
	GtkWidget *window;
	GtkWidget *bconnect;
	GtkWidget *bheip;
	GtkWidget *bip_list;
	GtkWidget *buser_num;
	GtkWidget *eport;
	GtkWidget *labelmenu;
}; 

// entry set: gtk_entry_set_text(GTK_ENTRY(data->ResultEntry),result_stringadd);
// entry get: const char *entrytemp=gtk_entry_get_text(GTK_ENTRY(data->ResultEntry));
// label set: gtk_label_set_text(GTK_LABEL(data->label), "CONNECT가 클릭됨!");

G_MODULE_EXPORT void quit(GtkWidget *window, gpointer data) {
	gtk_main_quit ();
}

// BUTTON CONNECT
G_MODULE_EXPORT void on_bconnect_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "CONNECT가 클릭됨!");
	const char *eport = gtk_entry_get_text(GTK_ENTRY(data->eport));
	printf("%s\n",eport);
}

// BUTTON HELP
G_MODULE_EXPORT void on_bhelp_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "HELP가 클릭됨!");
}

// BUTTON IP LIST
G_MODULE_EXPORT void on_bip_list_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "IP LIST가 클릭됨!");
}

// BUTTON USER NUMBER
G_MODULE_EXPORT void on_buser_num_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "USER NUMBER가 클릭됨!");
}

// BUTTON CHAT NUMBER
G_MODULE_EXPORT void on_bchat_num_clicked(GtkButton *button, Data *data) {
	gtk_label_set_text(GTK_LABEL(data->labelmenu), "CHAT NUMBER가 클릭됨!");
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


	gtk_builder_connect_signals (builder, data);
	g_object_unref(G_OBJECT(builder));
	gtk_widget_show_all(data->window);
	gtk_main ();
	g_slice_free (Data, data);
	return (0);
} 
