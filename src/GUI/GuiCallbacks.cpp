#include <GuiCallbacks.hpp>
#include <string>

/*
 *	PeersChar GUI Callback Functions
 *
 *	Note: Once GuiHandler's corresponding member function is called,
 *	      program control returns to GUI event loop.
 */

void activate_callback(GtkApplication *app, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	gh->activate(app);
}

void host_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	
	gchar *name_text;
	gchar *port_text;
	name_text = gh->get_child_entry_text(widget_box, "NameEntry");
	port_text = gh->get_child_entry_text(widget_box, "PortEntry");
	gh->set_user_name(name_text, gh->users.size());
	gh->set_user_port(port_text);
	PORT = (uint16_t) atoi(port_text);
	
	// host network
	if (Network->host() != TRUE)
	{
		printf("ERROR: Connection could not be established\n");
	}

	else
	{
		// start audio library
		gh->hostButtonPressed(widget, widget_box);
		Audio->startVoiceStream();
	}
}

void join_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	
	gchar *name_text;
	gchar *link_text;
	gchar *port_text;
	name_text = gh->get_child_entry_text(widget_box, "NameEntry");
	link_text = gh->get_child_entry_text(widget_box, "LinkEntry");
	port_text = gh->get_child_entry_text(widget_box, "PortEntry");
	gh->set_user_name(name_text, gh->users.size());
	gh->set_user_link(link_text);
	gh->set_user_port(port_text);

	// define socket address and join connection
	sockaddr_in addr; 
	if (inet_pton(AF_INET, link_text, &addr.sin_addr) < 1)
	{
		printf("ERROR: IP Address String is not a valid IPv4 Address.\n");
	}
	addr.sin_port = htons((uint16_t) atoi(port_text));	

	if (Network->join(addr) != TRUE)
	{
		printf("ERROR: Connection could not be established\n");
	}

	else
	{
		// start audio library
		gh->joinButtonPressed(widget, widget_box);
		Audio->startVoiceStream();
	}
}

void mute_button_callback(GtkWidget *widget)
{
	GtkWidget* list_row = gtk_widget_get_parent(widget);
	const gchar* name = gtk_widget_get_name(list_row);
	const std::string nameStr = name;
	NPeer* mute_peer = (*Network)[nameStr];
	if(mute_peer == NULL) {
		printf("ERROR: Could not find user to mute\n");
		return;
	}
	
	gboolean toggled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if(toggled)
	{
		g_print("Mute Button Toggled\n");
		// Call mute on mute_peer
	}
	else
	{
		g_print("Mute Button Untoggled\n");
		// Call unmute on mute_peer
	}
	g_print("Name of user: %s\n", name); 
}

void kick_button_callback(GtkWidget *widget, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* list_row = gtk_widget_get_parent(widget);
	const gchar* name = gtk_widget_get_name(list_row);
	
	g_print("Kick Button Pressed\n");
	g_print("Name of user: %s\n", name);
	
	gh->remove_name_from_session(name);
}

void volume_callback(GtkVolumeButton *v1, gdouble value)
{
	g_return_if_fail(GTK_IS_VOLUME_BUTTON(v1));
	
	Audio->setOutputVolume(value);
}

void leave_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->leaveButtonPressed(widget, widget_box);

	
	// disconnect from network
	Audio->stopVoiceStream();
	Network->disconnect();
}

