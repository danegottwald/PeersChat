#include <gtk/gtk.h>

// Callback function bound to button press
void greet(GtkWidget *widget, gpointer data)
{
        g_print("Hello World\n");
}

// Callback function run when application begins running, used instead of main()
void activate(GtkApplication *app, gpointer user_data)
{
	// Declare widget pointers
	GtkWidget *window;
    
	// Set up window
	window = gtk_application_window_new(app);
	
	gtk_widget_show_all(window);
}

int main(int argc, char *argv[])
{
	// Declare application pointer and return status
	GtkApplication *app;
	int status;
	
	// Create GtkApplication, bind and run through activate() callback
	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	
	// Free application from memory
	g_object_unref(app);
	
	return status;
}
