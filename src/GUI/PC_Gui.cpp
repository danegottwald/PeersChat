#include "GuiCallbacks.cpp"
#include "PC_Gui.hpp"


// Constructor
PC_GuiHandler::PC_GuiHandler()
{
	widget_box = NULL;
	name_list = NULL;
	
	user_name = NULL;
	user_link = NULL;	
	is_host = FALSE;

	app = gtk_application_new("edu.ucsc.PeersChat", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_callback), this);
}

// Destructor
PC_GuiHandler::~PC_GuiHandler()
{
	g_object_unref(app);
}

// Begins GUI event loop
int PC_GuiHandler::runGui(int argc, char *argv[])
{
	// Run GUI through GtkApplication
	int status;
	
	status = g_application_run(G_APPLICATION(app), argc, argv);
	
	g_object_unref(app);
	
	return status;
}

// Functions for adding/removing users to/from session

void PC_GuiHandler::add_host_to_session(const gchar *name)
{
	GtkWidget *new_row = create_new_user_row(name, TRUE, FALSE);
	gtk_container_add(GTK_CONTAINER(name_list), new_row);
}

void PC_GuiHandler::add_user_to_session(const gchar *name, bool kickable)
{
	GtkWidget *new_row = create_new_user_row(name, FALSE, kickable);
	gtk_container_add(GTK_CONTAINER(name_list), new_row);
}

void PC_GuiHandler::remove_name_from_session(const gchar *name)
{
	GList *rows = gtk_container_get_children(GTK_CONTAINER(name_list));
	
	while(rows != NULL)
	{
		GtkWidget *row = GTK_WIDGET(rows->data);
		// Row returns ListBoxRow obj, must get box inside
		GtkWidget *row_box = gtk_bin_get_child(GTK_BIN(row));
		const gchar* row_name = gtk_widget_get_name(row_box);
		if(strcmp(row_name, name) == 0)
		{
			gtk_container_remove(GTK_CONTAINER(name_list), row);
			break;
		}
		rows = rows->next;
	}
}

// GTK+ Callback functions bound to GtkObjects

void PC_GuiHandler::activate(GtkApplication *app, gpointer data)
{
	GtkWidget *window;
	GtkWidget *peerschat_label;
	GtkWidget *name_entry;
	GtkWidget *link_entry;
	GtkWidget *button_box;
	GtkWidget *host_button;
	GtkWidget *join_button;
	
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "PeersChat");
	gtk_window_set_default_size(GTK_WINDOW(window), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER (window), DEFAULT_WINDOW_PADDING);
	
	widget_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, DEFAULT_WIDGET_PADDING);
	gtk_container_add(GTK_CONTAINER(window), widget_box);
	
	peerschat_label = gtk_label_new("PeersChat: Demo Version");
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
	g_signal_connect(host_button, "clicked", G_CALLBACK(host_button_callback), this);
	gtk_container_add(GTK_CONTAINER(button_box), host_button);
	
	join_button = gtk_button_new_with_label("Join Session");
	g_signal_connect(join_button, "clicked", G_CALLBACK(join_button_callback), this);
	gtk_container_add(GTK_CONTAINER(button_box), join_button);

	// Pull focus away from text entries to display placeholder text;
	gtk_widget_grab_focus(host_button);
	
	gtk_widget_show_all(window);
}

void PC_GuiHandler::hostButtonPressed(GtkWidget *widget, gpointer data)
{	
	gchar *name_text;
	name_text = get_user_name();
	
	if(!entry_text_is_valid(name_text))
	{
		username_popup();
	}	
	else
	{
		set_user_name(name_text);
		
		is_host = TRUE;
		
		GtkWidget *lobby_box = NULL;
		setup_lobby(GTK_WIDGET(data), lobby_box);
	}
}

void PC_GuiHandler::joinButtonPressed(GtkWidget *widget, gpointer data)
{
	gchar *name_text;
	gchar *link_text;

	name_text = get_user_name();
	link_text = get_user_link();
	
	if(!entry_text_is_valid(name_text))
	{
		username_popup();
	}	
	else
	{
		set_user_name(name_text);
		set_user_link(link_text);
	
		is_host = FALSE;
		
		GtkWidget *lobby_box = NULL;
		setup_lobby(GTK_WIDGET(data), lobby_box);
	}
}

void PC_GuiHandler::outputVolChanged(GtkVolumeButton *v1, gdouble value, gpointer data)
{
	g_print("Value = %f\n", value);
}

void PC_GuiHandler::leaveButtonPressed(GtkWidget *widget, gpointer data)
{
	GtkWidget *lobby_box = get_widget_by_name(GTK_WIDGET(data), "LobbyBox");
	gtk_widget_destroy(lobby_box);
	gtk_widget_show_all(GTK_WIDGET(data));
}


// Utility functions used by GTK+ callback functions

// Public access functions

GtkWidget* PC_GuiHandler::get_widget_by_name(GtkWidget *container, const gchar *widget_name)
{
	GtkWidget *return_widget = NULL;
	GList *widgets = gtk_container_get_children(GTK_CONTAINER(container));
	
	while(widgets != NULL)
	{
		const gchar *child_name = gtk_widget_get_name(GTK_WIDGET(widgets->data));
		if(strcmp(widget_name, child_name) == 0)
		{
			return_widget = GTK_WIDGET(widgets->data);
			break;
		}
		widgets = widgets->next;
	}
	
	return return_widget;
}	

gchar* PC_GuiHandler::get_child_entry_text(GtkWidget *container, const gchar *entry_name)
{
	gchar *entry_text = NULL;

	GtkWidget *entry_widget = get_widget_by_name(container, entry_name);
	if(entry_widget != NULL) 
	{
		entry_text = const_cast<gchar*>(gtk_entry_get_text(GTK_ENTRY(entry_widget)));
	}
	
	return entry_text;
}

GtkWidget* PC_GuiHandler::get_widget_box()
{
	return widget_box;
}

void PC_GuiHandler::set_user_name(gchar *entry_text)
{
	user_name = entry_text;
}

gchar* PC_GuiHandler::get_user_name()
{
	return user_name;
}

void PC_GuiHandler::set_user_link(gchar *entry_text)
{
	user_link = entry_text;
}

gchar* PC_GuiHandler::get_user_link()
{
	return user_link;
}

// Private Utility Functions

void PC_GuiHandler::hide_all_child_widgets(GtkWidget *container)
{
	GList *widgets = gtk_container_get_children(GTK_CONTAINER(container));
	while(widgets != NULL) 
	{
		gtk_widget_hide(GTK_WIDGET(widgets->data));
		widgets = widgets->next;
	}
}

bool PC_GuiHandler::entry_text_is_valid(gchar *entry_text)
{
	bool match = regex_match(entry_text, std::regex("\\w+"));
	match = match && (strlen(entry_text) <= MAX_NAME_LEN);
	return match;
}

GtkWidget* PC_GuiHandler::create_new_user_row(const gchar *name, bool is_host, bool kickable)
{
	GtkWidget *new_row;
	GtkWidget *name_label;
	GtkWidget *mute_button;
	
	new_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_name(new_row, name);
	
	name_label = gtk_label_new(NULL);
	if(is_host)
	{
		const char *format = "<b>%s</b>";
		char *markup;
		
		markup = g_markup_printf_escaped(format, name);
		gtk_label_set_markup(GTK_LABEL(name_label), markup);
		g_free(markup);
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(name_label), name);
	}
	gtk_widget_set_name(name_label, "row_name");
	gtk_box_pack_start(GTK_BOX(new_row), name_label, FALSE, FALSE, FALSE);
	
	if(kickable)
	{
		GtkWidget *kick_button;
		kick_button = gtk_button_new_with_label("Kick");
		g_signal_connect(kick_button, "clicked", G_CALLBACK(kick_button_callback), this);
		gtk_box_pack_end(GTK_BOX(new_row), kick_button, FALSE, FALSE, FALSE);
	}
		
	mute_button = gtk_toggle_button_new_with_label("Mute");
	g_signal_connect(mute_button, "clicked", G_CALLBACK(mute_button_callback), this);
	gtk_box_pack_end(GTK_BOX(new_row), mute_button, FALSE, FALSE, FALSE);
	
	return new_row;
}

void PC_GuiHandler::setup_lobby(GtkWidget *parent, GtkWidget *lobby_box)
{
	hide_all_child_widgets(GTK_WIDGET(parent));
	lobby_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, DEFAULT_WIDGET_PADDING);
	gtk_widget_set_name(lobby_box, "LobbyBox");
	gtk_widget_set_vexpand(lobby_box, TRUE);
	gtk_container_add(GTK_CONTAINER(parent), lobby_box);

	name_list = gtk_list_box_new();
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(name_list), GTK_SELECTION_NONE);
	gtk_container_add(GTK_CONTAINER(lobby_box), name_list);

	GtkWidget *leave_button = gtk_button_new_with_label("Leave Session");
	gtk_box_pack_end(GTK_BOX(lobby_box), leave_button, FALSE, FALSE, 0);
	g_signal_connect(leave_button, "clicked", G_CALLBACK(leave_button_callback), this);

	// new box with output label and slider
	GtkWidget *outputBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DEFAULT_WIDGET_PADDING);
	gtk_widget_set_name(outputBox, "outputBox");
	gtk_widget_set_vexpand(outputBox, TRUE);
	gtk_box_pack_end(GTK_BOX(lobby_box), outputBox, FALSE, FALSE, 0);
	
	GtkWidget *outputVol = gtk_volume_button_new();
	gtk_box_pack_end(GTK_BOX(outputBox), outputVol, TRUE, FALSE, 0);
	g_signal_connect(outputVol, "value-changed", G_CALLBACK(volume_callback), this);

	GtkWidget *outputLabel = gtk_label_new((const gchar*) "Output Volume");
	gtk_box_pack_end(GTK_BOX(outputBox), outputLabel, FALSE, FALSE, 0);
	
	if(is_host) 
	{
		add_host_to_session(user_name);
		add_user_to_session("TestUser", TRUE);
	}
	else
	{
		add_user_to_session(user_name, FALSE);
		add_host_to_session("TestHost");
	}
	gtk_widget_show_all(lobby_box);
}

void PC_GuiHandler::show_error_popup(const gchar *message)
{
	GtkWidget *error_dialog;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	error_dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);

	gtk_dialog_run(GTK_DIALOG(error_dialog));
	gtk_widget_destroy(error_dialog);
}

void PC_GuiHandler::username_popup() 
{
	show_error_popup(
	"Error: Username restricted to (<= 18) alphanumeric characters.\n [A-Z], [a-z], [0-9], [_]" 
	);
}

