#include <gtk/gtk.h>


// Macros used for GtkWidget properties

const int DEFAULT_WINDOW_WIDTH = 360;
const int DEFAULT_WINDOW_HEIGHT = 240;

// Window padding: pads around border of window
const int DEFAULT_WINDOW_PADDING = 32;
// Widget padding: pads between widgets
const int DEFAULT_WIDGET_PADDING = 16;


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
  	GtkWidget *widget_box;	
	GtkWidget *peerschat_label;
	GtkWidget *name_entry;
	GtkWidget *link_entry;
	GtkWidget *button_box;
    GtkWidget *host_button;
	GtkWidget *join_button;
	
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "PeersChat GUI Demo");
	gtk_window_set_default_size(GTK_WINDOW(window), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER (window), DEFAULT_WINDOW_PADDING);
	
	widget_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, DEFAULT_WIDGET_PADDING);
	gtk_container_add(GTK_CONTAINER(window), widget_box);
	
	peerschat_label = gtk_label_new("PeersChat GUI Demo");
	gtk_container_add(GTK_CONTAINER(widget_box), peerschat_label);
	
	name_entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Enter Username");
	gtk_container_add(GTK_CONTAINER(widget_box), name_entry);
	
	link_entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(link_entry), "If Joining Session: Enter Link");
	gtk_container_add(GTK_CONTAINER(widget_box), link_entry);
	
	button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER(widget_box), button_box);
	
	host_button = gtk_button_new_with_label("Host Session");
	g_signal_connect(host_button, "clicked", G_CALLBACK(hostButtonPressed), NULL);
	gtk_container_add(GTK_CONTAINER(button_box), host_button);
	
	join_button = gtk_button_new_with_label("Join Session");
	g_signal_connect(join_button, "clicked", G_CALLBACK(joinButtonPressed), NULL);
	gtk_container_add(GTK_CONTAINER(button_box), join_button);
	
	// Pull focus away from text entries to display place holder text;
	gtk_widget_grab_focus(host_button);
	
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
