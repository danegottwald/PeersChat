#include <gtk/gtk.h>
#include <regex>

// Macros used for GtkWidget properties

const int DEFAULT_WINDOW_WIDTH = 360;
const int DEFAULT_WINDOW_HEIGHT = 240;

// Window padding: pads around border of window
const int DEFAULT_WINDOW_PADDING = 32;
// Widget padding: pads between widgets
const int DEFAULT_WIDGET_PADDING = 16;


// Utility functions used by GTK+ callback functions

gchar *get_child_entry_text(GtkWidget *container, const gchar *entry_name)
{
	gchar *entry_text = NULL;
	GList *widgets = gtk_container_get_children(GTK_CONTAINER(container));

	while(widgets != NULL) {
		const gchar *widget_name = gtk_widget_get_name(GTK_WIDGET(widgets->data));
		if(strcmp(widget_name, entry_name) == 0)
		{
			entry_text = const_cast<gchar*>(gtk_entry_get_text(GTK_ENTRY(widgets->data)));
			break;
		}
		widgets = widgets->next;
	}

	return entry_text;
}

bool entry_text_is_valid(gchar *entry_text)
{
	bool match = std::regex_match(entry_text, std::regex("\\w+"));
	return match;
}

void show_error_popup(const gchar *message)
{
	GtkWidget *error_dialog;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	error_dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);

	gtk_dialog_run(GTK_DIALOG(error_dialog));
	gtk_widget_destroy(error_dialog);
}

void username_popup() {
	show_error_popup(
	"Error: Username restricted to alphanumeric characters.\n [A-Z], [a-z], [0-9], [_]" 
	);
}


// GTK+ Callback functions bound to GtkObjects

void hostButtonPressed(GtkWidget *widget, gpointer data)
{
        g_print("Host Button pressed\n");
	
	gchar *name_text;

	name_text = get_child_entry_text(GTK_WIDGET(data), "NameEntry");
	g_print("Name Entry Text: %s\n", name_text);
	
	if(!entry_text_is_valid(name_text))
	{
		username_popup();
	}	
	else
	{
		// Begin hosting session
	}
}

void joinButtonPressed(GtkWidget *widget, gpointer data)
{
        g_print("Join Button pressed\n");

	gchar *name_text;
	gchar *link_text;

	name_text = get_child_entry_text(GTK_WIDGET(data), "NameEntry");
	link_text = get_child_entry_text(GTK_WIDGET(data), "LinkEntry");	

	g_print("Name Entry Text: %s\n", name_text);
	g_print("Link Entry Text: %s\n", link_text);
	
	if(!entry_text_is_valid(name_text))
	{
		username_popup();
	}	
	else
	{
		// Begin joining session
	}
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
	GtkWidget *error_dialog;
	
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "PeersChat GUI Demo");
	gtk_window_set_default_size(GTK_WINDOW(window), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER (window), DEFAULT_WINDOW_PADDING);
	
	widget_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, DEFAULT_WIDGET_PADDING);
	gtk_container_add(GTK_CONTAINER(window), widget_box);
	
	peerschat_label = gtk_label_new("PeersChat GUI Demo");
	gtk_container_add(GTK_CONTAINER(widget_box), peerschat_label);
	
	name_entry = gtk_entry_new();
	gtk_widget_set_name(name_entry, "NameEntry");
	gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Enter Username");
	gtk_container_add(GTK_CONTAINER(widget_box), name_entry);
	
	link_entry = gtk_entry_new();
	gtk_widget_set_name(link_entry, "LinkEntry");
	gtk_entry_set_placeholder_text(GTK_ENTRY(link_entry), "If Joining Session: Enter Link");
	gtk_container_add(GTK_CONTAINER(widget_box), link_entry);
	
	button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER(widget_box), button_box);
	
	host_button = gtk_button_new_with_label("Host Session");
	g_signal_connect(host_button, "clicked", G_CALLBACK(hostButtonPressed), widget_box);
	gtk_container_add(GTK_CONTAINER(button_box), host_button);
	
	join_button = gtk_button_new_with_label("Join Session");
	g_signal_connect(join_button, "clicked", G_CALLBACK(joinButtonPressed), widget_box);
	gtk_container_add(GTK_CONTAINER(button_box), join_button);
	
	

	// Pull focus away from text entries to display placeholder text;
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
