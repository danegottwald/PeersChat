#include <gtk/gtk.h>

// Callback function bound to button press
void greet(GtkWidget *widget, gpointer data)
{
	g_print("Hello World\n");
}

// Callback function to close window when x'ed out
void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *button;
	
	// Init GTK+ libraries
	gtk_init (&argc, &argv);


	// Create window (pointer assignment)
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	// function binds event to callback function
	g_signal_connect(window, "destroy", G_CALLBACK (destroy), NULL);
	
	// Adjust window properties
	gtk_container_set_border_width(GTK_CONTAINER (window), 20);
	gtk_window_set_title (GTK_WINDOW (window), "GTK TEST");
	gtk_window_set_default_size(GTK_WINDOW (window), 256, 128);
	
	
	// Create button, bind, add to window 
	button = gtk_button_new_with_label ("Click here");
	
	g_signal_connect(GTK_WIDGET(button), "clicked", G_CALLBACK (greet), button);
	
	gtk_container_add (GTK_CONTAINER (window), button);
	
	
	// Display window and begin GUI interaction
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
