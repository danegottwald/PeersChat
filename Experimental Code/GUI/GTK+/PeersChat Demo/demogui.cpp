#include <gtk/gtk.h>


// GTK+ Callback functions bound to GtkObjects

void hostButtonPressed(GtkWidget *widget, gpointer data)
{
        g_print("Host Button pressed\n");
}

void joinButtonPressed(GtkWidget *widget, gpointer data)
{
        g_print("Join Button pressed\n");
}

void activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *button_box;
    GtkWidget *host_button;
	GtkWidget *join_button;
	
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "PeersChat GUI Demo");
	gtk_window_set_default_size(GTK_WINDOW(window), 360, 240);
	gtk_container_set_border_width(GTK_CONTAINER (window), 32);
	
	button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER(window), button_box);
	
	host_button = gtk_button_new_with_label("Host Session");
	g_signal_connect(host_button, "clicked", G_CALLBACK(hostButtonPressed), NULL);
	gtk_container_add(GTK_CONTAINER(button_box), host_button);
	
	join_button = gtk_button_new_with_label("Join Session");
	g_signal_connect(join_button, "clicked", G_CALLBACK(joinButtonPressed), NULL);
	gtk_container_add(GTK_CONTAINER(button_box), join_button);
	
	gtk_widget_show_all(window);
}

int main(int argc, char *argv[])
{
	// Run GUI through GtkApplication
	GtkApplication *app;
	int status;
	
	app = gtk_application_new("edu.ucsc.PeersChat", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	
	g_object_unref(app);
	
	return status;
}
