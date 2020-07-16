#include <gtk/gtk.h>
#include <string.h>
extern "C" void on_button1_clicked (GtkButton *b);

GtkWidget *label1;

int main(int argc, char *argv[])
{	
	GtkBuilder *builder;
	GtkWidget *window;
	GtkWidget *fixed1;
	GtkWidget *button1;

	// init gtk lib
	gtk_init(&argc, &argv);

	// add glade files
	builder = gtk_builder_new_from_file("bitch.glade");

	// get myWindow widget
	window = GTK_WIDGET(gtk_builder_get_object(builder, "myWindow"));
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_builder_connect_signals(builder, NULL);

	fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
	button1 = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
	label1 = GTK_WIDGET(gtk_builder_get_object(builder, "label1"));
	g_object_unref(G_OBJECT( builder ) );

	// show window
	gtk_widget_show(window);
	gtk_main();

	return 0;
}

extern "C" void on_button1_clicked (GtkButton *b)
{
	gtk_label_set_text(GTK_LABEL(label1), (const gchar* ) "Fuck you");
}
