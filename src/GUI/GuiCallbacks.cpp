#include "PC_Gui.hpp"
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
	gh->activate(app, NULL);
}

void host_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	
	gchar *name_text;
	name_text = gh->get_child_entry_text(widget_box, "NameEntry");
	gh->set_user_name(name_text);
	
	gh->hostButtonPressed(widget, widget_box);


	// host network
	if (Network->host() != TRUE)
	{
		// throw error
	}
}

void join_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	
	gchar *name_text;
	gchar *link_text;
	name_text = gh->get_child_entry_text(widget_box, "NameEntry");
	link_text = gh->get_child_entry_text(widget_box, "LinkEntry");
	gh->set_user_name(name_text);
	gh->set_user_link(link_text);
	
	gh->joinButtonPressed(widget, widget_box);


	// parse link_text for IP address and port
	std::string str(link_text);
	size_t pos = str.find_first_of(':', 0);
	std::string ip = str.substr(0, pos - 1);
	std::string port = str.substr(pos, 10);

	// define socket address and join connection
	sockaddr_in addr; 
	if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) < 1)
	{
		// throw error
	}
	addr.sin_port = htons((uint16_t) atoi(port.c_str()));	

	if (Network->join(addr) != TRUE)
	{
		// signal error joining call
	}
}

void mute_button_callback(GtkWidget *widget, gpointer data)
{
	GtkWidget* list_row = gtk_widget_get_parent(widget);
	const gchar* name = gtk_widget_get_name(list_row);
	
	g_print("Mute Button Pressed\n");
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

void volume_callback(GtkVolumeButton *v1, gdouble value, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->outputVolChanged(v1, value, widget_box);
}

void leave_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->leaveButtonPressed(widget, widget_box);

	
	// disconnect from network
	Network->disconnect();
}

