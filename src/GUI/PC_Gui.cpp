#include <PC_Gui.hpp>

extern PeersChatNetwork *Network;
extern std::unique_ptr<std::thread> DISCONNECT_THREAD;


// Constructor
PC_GuiHandler::PC_GuiHandler()
{
	widget_box = NULL;
	name_list = NULL;

	user_name = NULL;
	user_link = NULL;
	user_port = NULL;
	is_host = FALSE;

	// Generate Unique Name by appending unix time in ms
	char name[100];
	sprintf(name, "edu.ucsc.PeersChat%" PRIu64, (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count());

	// Create GTK Application
	app = gtk_application_new(name, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_callback), this);
}

// Destructor
PC_GuiHandler::~PC_GuiHandler()
{

	// Optimize Leave Button Destructor
	if(DISCONNECT_THREAD.get() && DISCONNECT_THREAD->joinable())
		DISCONNECT_THREAD->join();
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

void PC_GuiHandler::add_self_to_session(const gchar *name)
{
	GtkWidget *new_row = create_new_user_row(name, FALSE, FALSE);
	gtk_widget_set_name(new_row, "UserRow");
	gtk_container_add(GTK_CONTAINER(name_list), new_row);
}

void PC_GuiHandler::add_user_to_session(const gchar *name, bool kickable)
{
	GtkWidget *new_row = create_new_user_row(name, FALSE, kickable);
	gtk_container_add(GTK_CONTAINER(name_list), new_row);
}

void PC_GuiHandler::add_user_to_session(const gchar *name, int id, bool kickable)
{
	GtkWidget *new_row = create_new_user_row(name, FALSE, kickable);
	std::string IDstr = std::to_string(id);
	gtk_widget_set_name(new_row, IDstr.c_str());
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

void PC_GuiHandler::add_npeer_to_gui(NPeer* peer)
{
	g_print("Npeer added to gui\n");
	users.push_back(peer);
	add_user_to_session(peer->getName().c_str(), peer->getID(), FALSE);
}

void PC_GuiHandler::remove_npeer_from_gui(NPeer* peer)
{
	std::vector<NPeer*>::iterator users_iter;
	for(users_iter = users.begin(); users_iter != users.end(); ++users_iter)
	{
		if(*users_iter == peer)
		{
			const gchar *row_id = std::to_string(peer->getID()).c_str();
			remove_name_from_session(row_id);
			users.erase(users_iter);
			break;
		}
	}
}

void PC_GuiHandler::refresh_name_list()
{
	g_print("Refreshed name list\n");
	GList *list_rows = gtk_container_get_children(GTK_CONTAINER(name_list));
	while(list_rows != NULL)
	{
		GtkWidget* current_row = gtk_bin_get_child(GTK_BIN(list_rows->data));
		const gchar* row_id = gtk_widget_get_name(current_row);

		// Disregard User's Row
		if(strcmp(row_id, "UserRow") == 0)
		{
			list_rows = list_rows->next;
			continue;
		}

		NPeer* id_peer = get_npeer(atoi(row_id));

		if(id_peer != NULL)
			rename_user_row(current_row, (id_peer->getName()).c_str());

		list_rows = list_rows->next;
	}

	gtk_widget_show_all(name_list);
}


// GTK+ Callback functions bound to GtkObjects

void PC_GuiHandler::activate(GtkApplication *app)
{
	GtkWidget *window;
	GtkWidget *peerschat_label;
	GtkWidget *name_entry;
	GtkWidget *entry_box;
	GtkWidget *link_entry;
	GtkWidget *port_entry;
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

	entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_name(entry_box, "EntryBox");
	gtk_container_add(GTK_CONTAINER(widget_box), entry_box);

	link_entry = gtk_entry_new();
	gtk_widget_set_name(link_entry, "LinkEntry");
	gtk_entry_set_placeholder_text(GTK_ENTRY(link_entry), "Enter (IP:HostPort) for Joining");
	gtk_box_pack_start(GTK_BOX(entry_box), link_entry, TRUE, TRUE, 0);

	port_entry = gtk_entry_new();
	gtk_widget_set_name(port_entry, "PortEntry");
	gtk_entry_set_placeholder_text(GTK_ENTRY(port_entry), "Port#");
	gtk_box_pack_end(GTK_BOX(entry_box), port_entry, FALSE, FALSE, 0);

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
	g_return_if_fail(GTK_IS_BUTTON(widget));

	gchar *name_text;
	name_text = get_user_name();

	if(!entry_text_is_valid(name_text) || strlen(name_text) < 1)
	{
		username_popup();
	}

	else
	{

		is_host = TRUE;

		GtkWidget *lobby_box = NULL;
		setup_lobby(GTK_WIDGET(data), lobby_box);
	}
}

void PC_GuiHandler::joinButtonPressed(GtkWidget *widget, gpointer data)
{
	g_return_if_fail(GTK_IS_BUTTON(widget));

	gchar *name_text;

	name_text = get_user_name();

	if(!entry_text_is_valid(name_text) || strlen(name_text) < 1)
	{
		username_popup();
	}
	else
	{

		is_host = FALSE;

		GtkWidget *lobby_box = NULL;
		setup_lobby(GTK_WIDGET(data), lobby_box);
		
	}
}

void PC_GuiHandler::leaveButtonPressed(GtkWidget *widget, gpointer data)
{
	g_return_if_fail(GTK_IS_BUTTON(widget));

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
	else
	{
		GtkWidget *entry_box = get_widget_by_name(container, "EntryBox");
		if(entry_box != NULL)
		{
			entry_widget = get_widget_by_name(entry_box, entry_name);
			if(entry_widget != NULL)
			{
				entry_text = const_cast<gchar*>(gtk_entry_get_text(GTK_ENTRY(entry_widget)));
			}
		}
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

void PC_GuiHandler::set_user_port(gchar *entry_text)
{
	user_port = entry_text;
	PORT = (uint16_t) atoi(entry_text);
}

gchar* PC_GuiHandler::get_user_port()
{
	return user_port;
}

NPeer* PC_GuiHandler::get_npeer(const gchar *peer_name)
{
	NPeer* return_peer = NULL;
	for(size_t user_index = 0; user_index < users.size(); ++user_index)
	{
		NPeer* current_peer = users[user_index];
		gchar *current_name = const_cast<gchar*>((current_peer->getName()).c_str());
		if(strcmp(current_name, peer_name) == 0)
		{
			return_peer = current_peer;
			break;
		}
	}
	return return_peer;
}

NPeer* PC_GuiHandler::get_npeer(const int id)
{
	NPeer* return_peer = NULL;
	for(size_t user_index = 0; user_index < users.size(); ++user_index)
	{
		NPeer* current_peer = users[user_index];
		const int current_id = current_peer->getID();
		if(id == current_id)
		{
			return_peer = current_peer;
			break;
		}
	}
	return return_peer;
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

void PC_GuiHandler::rename_user_row(GtkWidget* row, const gchar* name)
{
	GtkWidget* row_label = get_widget_by_name(row, "row_name");
	gtk_label_set_text(GTK_LABEL(row_label), name);
}

void PC_GuiHandler::setup_lobby(GtkWidget *parent, GtkWidget *lobby_box)
{
	g_print("Setup lobby called\n");	
	
	hide_all_child_widgets(GTK_WIDGET(parent));
	lobby_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, DEFAULT_WIDGET_PADDING);
	gtk_widget_set_name(lobby_box, "LobbyBox");
	gtk_widget_set_vexpand(lobby_box, TRUE);
	gtk_container_add(GTK_CONTAINER(parent), lobby_box);

	std::string port_string = "TCP/UDP listening on port: " + std::to_string(PORT);
	GtkWidget *port_label = gtk_label_new(port_string.c_str());
	gtk_container_add(GTK_CONTAINER(lobby_box), port_label);

	name_list = gtk_list_box_new();
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(name_list), GTK_SELECTION_NONE);
	gtk_container_add(GTK_CONTAINER(lobby_box), name_list);

	GtkWidget *leave_button = gtk_button_new_with_label("Leave Session");
	gtk_box_pack_end(GTK_BOX(lobby_box), leave_button, FALSE, FALSE, 0);
	g_signal_connect(leave_button, "clicked", G_CALLBACK(leave_button_callback), this);

	GtkWidget *volumeSlider = create_volume_slider();
	gtk_box_pack_end(GTK_BOX(lobby_box), volumeSlider, FALSE, FALSE, 0);

	GtkWidget *indirectToggle = create_indirect_join_toggle();
	gtk_box_pack_end(GTK_BOX(lobby_box), indirectToggle, FALSE, FALSE, 0);

	GtkWidget *directToggle = create_direct_join_toggle();
	gtk_box_pack_end(GTK_BOX(lobby_box), directToggle, FALSE, FALSE, 0);

	add_self_to_session(user_name);

	gtk_widget_show_all(lobby_box);
}

GtkWidget* PC_GuiHandler::create_volume_slider()
{
	GtkWidget *outputBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DEFAULT_WIDGET_PADDING);
	gtk_widget_set_name(outputBox, "outputBox");
	gtk_widget_set_vexpand(outputBox, TRUE);

	GtkWidget *outputVol = gtk_volume_button_new();
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(outputVol), DEFAULT_VOLUME_SLIDER_VALUE);
	gtk_box_pack_end(GTK_BOX(outputBox), outputVol, TRUE, FALSE, 0);
	g_signal_connect(outputVol, "value-changed", G_CALLBACK(volume_callback), this);

	GtkWidget *outputLabel = gtk_label_new((const gchar*) "Output Volume          ");
	gtk_box_pack_end(GTK_BOX(outputBox), outputLabel, FALSE, FALSE, 0);

	return outputBox;
}

GtkWidget* PC_GuiHandler::create_direct_join_toggle()
{
	GtkWidget *directBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DEFAULT_WIDGET_PADDING);
	gtk_widget_set_vexpand(directBox, TRUE);

	GtkWidget *directCheck = gtk_check_button_new();
	gtk_box_pack_end(GTK_BOX(directBox), directCheck, TRUE, FALSE, 0);
	g_signal_connect(directCheck, "clicked", G_CALLBACK(direct_checkmark_callback), this);

	GtkWidget *directLabel = gtk_label_new((const gchar*) "Allow Direct Joins    ");
	gtk_box_pack_end(GTK_BOX(directBox), directLabel, FALSE, FALSE, 0);

	return directBox;
}

GtkWidget* PC_GuiHandler::create_indirect_join_toggle()
{
	GtkWidget *indirectBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DEFAULT_WIDGET_PADDING);
	gtk_widget_set_vexpand(indirectBox, TRUE);

	GtkWidget *indirectCheck = gtk_check_button_new();
	gtk_box_pack_end(GTK_BOX(indirectBox), indirectCheck, TRUE, FALSE, 0);
	g_signal_connect(indirectCheck, "clicked", G_CALLBACK(indirect_checkmark_callback), this);

	GtkWidget *indirectLabel = gtk_label_new((const gchar*) "Allow Indirect Joins");
	gtk_box_pack_end(GTK_BOX(indirectBox), indirectLabel, FALSE, FALSE, 0);

	return indirectBox;
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
	"Error: Username restricted to (1-18) alphanumeric characters.\n [A-Z], [a-z], [0-9], [_]"
	);
}

